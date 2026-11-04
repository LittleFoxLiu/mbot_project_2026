#include "ColourSensor.h"
#include "Lights.h"
#include "LineSensor.h"
#include "Movement.h"
#include "Celebration.h"
#include "RobotConfig.h"
#include <math.h>

struct ColourRef
{
    int code;
    int red, green, blue; // Raw calibration readings.
};

// Averaged raw readings from each test card. Re-measure after hardware changes.
static const ColourRef COLOUR_REFS[] = {
    {COLOUR_RED,        951, 800, 822},
    {COLOUR_RED,        951, 800, 822},
    {COLOUR_RED,        951, 800, 822},
    {COLOUR_GREEN,      879, 910, 887},
    {COLOUR_GREEN,      879, 901, 889},
    {COLOUR_GREEN,      879, 910, 887},
    {COLOUR_ORANGE,     957, 876, 861},
    {COLOUR_ORANGE,     957, 876, 861},
    {COLOUR_ORANGE,     957, 876, 861},
    {COLOUR_PINK,       959, 916, 930},
    {COLOUR_PINK,       959, 916, 930},
    {COLOUR_PINK,       959, 916, 930},
    {COLOUR_LIGHT_BLUE, 862, 890, 928},
    {COLOUR_LIGHT_BLUE, 862, 890, 928},
    {COLOUR_LIGHT_BLUE, 862, 890, 928},
    {COLOUR_WHITE,      960, 943, 951},
    {COLOUR_WHITE,      960, 943, 951},
    {COLOUR_WHITE,      960, 943, 951},
};

static const uint8_t COLOUR_REF_COUNT = sizeof(COLOUR_REFS) / sizeof(COLOUR_REFS[0]);
static const uint8_t COLOUR_CLASS_COUNT = COLOUR_WHITE + 1;

// Derive K from the number of calibration points per colour (n - 1).
// One point per colour gives K=0, so use the valid nearest-neighbour fallback K=1.
static uint8_t getKnnK()
{
    uint8_t pointsPerColour = COLOUR_REF_COUNT / COLOUR_CLASS_COUNT;
    uint8_t k = pointsPerColour > 0 ? pointsPerColour - 1 : 0;
    return k > 0 ? k : 1;
}

const char *colourName(int code)
{
    switch (code)
    {
    case COLOUR_RED: return "RED";
    case COLOUR_GREEN: return "GREEN";
    case COLOUR_ORANGE: return "ORANGE";
    case COLOUR_PINK: return "PINK";
    case COLOUR_LIGHT_BLUE: return "LIGHT_BLUE";
    case COLOUR_WHITE: return "WHITE";
    default: return "UNKNOWN";
    }
}

static int readColourChannel(void (*shine)());

static void readColourRaw(int &red, int &green, int &blue)
{
    red = readColourChannel(shineRed);
    green = readColourChannel(shineGreen);
    blue = readColourChannel(shineBlue);
    shineIR();
}

static int readColourChannel(void (*shine)())
{
    shine();
    waitWithLineMonitoring(COLOUR_SETTLE_MS);
    // Averaging keeps the same mean as the single-read calibration but less noise.
    long sum = 0;
    for (uint8_t i = 0; i < COLOUR_SAMPLES; i++)
        sum += analogRead(LDR_PIN);
    return sum / COLOUR_SAMPLES;
}

// Scale a raw reading so black is 0 and white is 1.
static double normalise(int raw, int black, int white)
{
    return (double)(raw - black) / (white - black);
}

int detectColour()
{
    int red, green, blue;
    readColourRaw(red, green, blue);

    Serial.print("R: ");
    Serial.print(red);
    Serial.print(" G: ");
    Serial.print(green);
    Serial.print(" B: ");
    Serial.println(blue);

    double r = normalise(red, RED_BLACK, RED_WHITE);
    double g = normalise(green, GREEN_BLACK, GREEN_WHITE);
    double b = normalise(blue, BLUE_BLACK, BLUE_WHITE);

    // K-nearest neighbours, with K derived from calibration points per colour.
    const uint8_t k = getKnnK();
    double distances[COLOUR_REF_COUNT];
    for (uint8_t index = 0; index < COLOUR_REF_COUNT; index++)
    {
        const ColourRef &ref = COLOUR_REFS[index];
        double dr = r - normalise(ref.red, RED_BLACK, RED_WHITE);
        double dg = g - normalise(ref.green, GREEN_BLACK, GREEN_WHITE);
        double db = b - normalise(ref.blue, BLUE_BLACK, BLUE_WHITE);
        distances[index] = dr * dr + dg * dg + db * db;
    }

    uint8_t votes[COLOUR_CLASS_COUNT] = {};
    double nearestDistSq = MAX_COLOUR_DISTANCE * MAX_COLOUR_DISTANCE;
    for (uint8_t neighbour = 0; neighbour < k; neighbour++)
    {
        int nearestIndex = -1;
        double nearest = 1.0e30;
        for (uint8_t i = 0; i < COLOUR_REF_COUNT; i++)
        {
            if (distances[i] < nearest)
            {
                nearest = distances[i];
                nearestIndex = i;
            }
        }
        if (nearestIndex < 0)
            break;
        if (neighbour == 0)
            nearestDistSq = nearest;
        votes[COLOUR_REFS[nearestIndex].code]++;
        distances[nearestIndex] = 1.0e30; // Do not select the same point twice.
    }

    if (nearestDistSq > MAX_COLOUR_DISTANCE * MAX_COLOUR_DISTANCE)
    {
        Serial.print("Colour KNN: ");
        Serial.print(colourName(COLOUR_UNKNOWN));
        Serial.print(" (K=");
        Serial.print(k);
        Serial.print(", nearest distance=");
        Serial.print(sqrt(nearestDistSq), 4);
        Serial.println(")");
        return COLOUR_UNKNOWN;
    }

    int best = COLOUR_UNKNOWN;
    uint8_t bestVotes = 0;
    for (uint8_t colour = 0; colour < COLOUR_CLASS_COUNT; colour++)
    {
        if (votes[colour] > bestVotes)
        {
            bestVotes = votes[colour];
            best = colour;
        }
    }
    Serial.print("Colour KNN: ");
    Serial.print(colourName(best));
    Serial.print(" (K=");
    Serial.print(k);
    Serial.print(", nearest distance=");
    Serial.print(sqrt(nearestDistSq), 4);
    Serial.println(")");
    return best;
}

int readWaypointColour()
{
    while (true)
    {
        int colour = detectColour();
        Serial.print("Colour code: ");
        Serial.println(colour);
        if (colour != COLOUR_UNKNOWN)
            return colour;
        delay(COLOUR_RETRY_MS);
    }
}

void actOnColour(int colour)
{
    switch (colour)
    {
    case COLOUR_RED:
        Serial.println("Red: turn left");
        turnLeft();
        break;
    case COLOUR_GREEN:
        Serial.println("Green: turn right");
        turnRight();
        break;
    case COLOUR_ORANGE:
        Serial.println("Orange: U-turn (two left turns)");
        uTurn();
        break;
    case COLOUR_PINK:
        Serial.println("Pink: left, forward one grid, left");
        doubleLeftTurn();
        break;
    case COLOUR_LIGHT_BLUE:
        Serial.println("Blue: right, forward one grid, right");
        doubleRightTurn();
        break;
    case COLOUR_WHITE:
        Serial.println("White: finish");
        celebrate();
        while (true)
            ; // Finished: stay stopped until reset.
    }
}

void runColourSensorReadoutMode()
{
    Serial.println();
    Serial.println("=== COLOUR SENSOR READOUT ===");
    while (true)
    {
        int red, green, blue;
        readColourRaw(red, green, blue);
        Serial.print("R=");
        Serial.print(red);
        Serial.print("  G=");
        Serial.print(green);
        Serial.print("  B=");
        Serial.println(blue);
        delay(100);
    }
}
