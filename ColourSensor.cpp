#include "ColourSensor.h"
#include "Lights.h"
#include "LineSensor.h"
#include "Movement.h"
#include "Celebration.h"
#include "RobotConfig.h"
#include <math.h>

static const uint8_t LDR_PIN = A3; // Port 3, S2.

// Configure the light-dependent resistor input.
void setupColourSensor()
{
    pinMode(LDR_PIN, INPUT);
}

static const double MAX_COLOUR_DISTANCE = 0.2; // Readings further than this from every reference return unknown.

struct ColourRef
{
    int code;
    int red, green, blue; // Raw calibration readings.
};

static const ColourRef COLOUR_REFS[] = {
    COLOUR_CALIBRATION_POINTS
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

// Return a readable label for a colour code, including unknown values.
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

// Read each channel under its matching LED, then restore the IR emitter.
static void readColourRaw(int &red, int &green, int &blue)
{
    red = readColourChannel(shineRed);
    green = readColourChannel(shineGreen);
    blue = readColourChannel(shineBlue);
    shineIR();
}

// Settle the selected LED and average the configured number of LDR samples.
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

// Map a raw channel value onto the black-to-white calibration scale.
static double normalise(int raw, int black, int white)
{
    return (double)(raw - black) / (white - black);
}

// Classify a waypoint by normalized RGB distance and K-nearest-neighbour voting.
int detectColour()
{
    int red, green, blue;
    readColourRaw(red, green, blue);

    if (DEBUG_MODE)
    {
        Serial.print("R: ");
        Serial.print(red);
        Serial.print(" G: ");
        Serial.print(green);
        Serial.print(" B: ");
        Serial.println(blue);
    }

    double r = normalise(red, RED_BLACK, RED_WHITE);
    double g = normalise(green, GREEN_BLACK, GREEN_WHITE);
    double b = normalise(blue, BLUE_BLACK, BLUE_WHITE);

    // Compare normalized channels so different raw calibration ranges are comparable.
    // Squared Euclidean RGB distance preserves nearest-point ordering without square roots.
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
    // Select K distinct nearest points and count their votes by colour class.
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

    // Reject samples outside the allowed radius even if one calibration point is closest.
    if (nearestDistSq > MAX_COLOUR_DISTANCE * MAX_COLOUR_DISTANCE)
    {
        if (DEBUG_MODE)
        {
            Serial.print("Colour KNN: ");
            Serial.print(colourName(COLOUR_UNKNOWN));
            Serial.print(" (K=");
            Serial.print(k);
            Serial.print(", nearest distance=");
            Serial.print(sqrt(nearestDistSq), 4);
            Serial.println(")");
        }
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
    if (DEBUG_MODE)
    {
        Serial.print("Colour KNN: ");
        Serial.print(colourName(best));
        Serial.print(" (K=");
        Serial.print(k);
        Serial.print(", nearest distance=");
        Serial.print(sqrt(nearestDistSq), 4);
        Serial.println(")");
    }
    return best;
}

// Retry sensor classification until a known colour is returned.
int readWaypointColour()
{
    while (true)
    {
        int colour = detectColour();
        if (DEBUG_MODE)
        {
            Serial.print("Colour code: ");
            Serial.println(colour);
        }
        if (colour != COLOUR_UNKNOWN)
            return colour;
        delay(COLOUR_RETRY_MS);
    }
}

// Dispatch a recognized waypoint colour to its movement or finish action.
void actOnColour(int colour)
{
    switch (colour)
    {
    case COLOUR_RED:
        if (DEBUG_MODE) Serial.println("Red: turn left");
        turnLeft();
        break;
    case COLOUR_GREEN:
        if (DEBUG_MODE) Serial.println("Green: turn right");
        turnRight();
        break;
    case COLOUR_ORANGE:
        if (DEBUG_MODE) Serial.println("Orange: U-turn (two left turns)");
        uTurn();
        break;
    case COLOUR_PINK:
        if (DEBUG_MODE) Serial.println("Pink: left, forward one grid, left");
        doubleLeftTurn();
        break;
    case COLOUR_LIGHT_BLUE:
        if (DEBUG_MODE) Serial.println("Blue: right, forward one grid, right");
        doubleRightTurn();
        break;
    case COLOUR_WHITE:
        if (DEBUG_MODE) Serial.println("White: finish");
        if (!(DEBUG_MODE && COLOUR_SENSOR_READOUT_MODE))
            celebrate();
        while (true)
            ; // Finished: stay stopped until reset.
    }
}

// Continuously print raw RGB samples for sensor calibration diagnostics.
void runColourSensorReadoutMode()
{
    if (!DEBUG_MODE)
        return;

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
