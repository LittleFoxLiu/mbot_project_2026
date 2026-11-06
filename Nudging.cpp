#include "Nudging.h"
#include "Hardware.h"
#include "Lights.h"
#include "RobotConfig.h"

static const uint8_t IR_SENSOR_PIN = A2; // Port 3, S1.

static int currentNudge = 0;
static bool ultrasonicNudgeActive = false;
static unsigned long ultrasonicNudgeStartedMs = 0;
static unsigned long ultrasonicNudgeEndedMs = 0;
static bool haveUltrasonicNudged = false;
static bool irNudgeActive = false;
static int irStartupReading = 0;
static unsigned long irNudgeStartedMs = 0;
static unsigned long irNudgeEndedMs = 0;
static bool haveIRNudged = false;
static const unsigned long ULTRASONIC_NUDGE_MS =
    (((850UL * 100 + TURN_SPEED / 2) / TURN_SPEED) * ULTRASONIC_NUDGE_ANGLE_DEG + 45) / 90;

void setupIRReference()
{
    // Capture a fixed IR-on reference once at startup, with motors stopped.
    pinMode(IR_SENSOR_PIN, INPUT);
    shineIR();
    delay(IR_SETTLE_MS);
    irStartupReading = analogRead(IR_SENSOR_PIN);
    if (DEBUG_MODE)
    {
        Serial.print("IR startup reference: ");
        Serial.println(irStartupReading);
    }
}

int forwardNudge(int speed)
{
    // updateNudging() selects only one sensor's correction at a time.
    // Reduce steering with the base speed during slowdown to avoid reversing a wheel.
    return (long)currentNudge * speed / FORWARD_SPEED;
}

bool isIRNudgeActive()
{
    return irNudgeActive;
}

// Returns the IR reading so the debug output can show the drop.
static int nudgeLeft()
{
    unsigned long now = millis();
    // The emitter stays on while driving; detectColour() switches back to it.
    int illuminated = analogRead(IR_SENSOR_PIN);

    // Compare against the fixed startup reading, not a changing ambient reference.
    // Trigger fixed left-turn pulses with a pause; strength does not depend on distance.
    if (IR_DROP_THRESHOLD >= 0 && irStartupReading - illuminated >= IR_DROP_THRESHOLD
        && !irNudgeActive
        && (!haveIRNudged || now - irNudgeEndedMs >= IR_NUDGE_PAUSE_MS))
    {   
        irNudgeActive = true;
        haveIRNudged = true;
        irNudgeStartedMs = now;
    }
    return illuminated;
}

static double nudgeRight()
{
    double distanceCm = ultrasonic.distanceCm();

    // A missing/out-of-range echo cannot provide a usable wall distance.
    if (distanceCm <= 0 || distanceCm >= 400)
    {
        return distanceCm;
    }
    unsigned long now = millis();
    if (distanceCm < ULTRASONIC_RIGHT_NUDGE_DISTANCE_CM
        && !ultrasonicNudgeActive
        && (!haveUltrasonicNudged || now - ultrasonicNudgeEndedMs >= ULTRASONIC_NUDGE_PAUSE_MS))
    {
        ultrasonicNudgeActive = true;
        haveUltrasonicNudged = true;
        ultrasonicNudgeStartedMs = now;
    }
    return distanceCm;
}

// Shows raw sensor readings and the correction that would be applied.
static void printNudgeDebug(double distanceCm, int irReading)
{
    static unsigned long lastPrintMs = 0;
    static bool havePrinted = false;
    unsigned long now = millis();
    if (havePrinted && now - lastPrintMs < DEBUG_NUDGING_PRINT_INTERVAL_MS)
        return;
    lastPrintMs = now;
    havePrinted = true;

    Serial.print("US: ");
    Serial.print(distanceCm);
    Serial.print(" cm");
    if (irReading >= 0)
    {
        Serial.print("  IR raw: ");
        Serial.print(irReading);
    }
    Serial.print("  nudge: ");
    if (irNudgeActive)
        Serial.println("left (IR)");
    else if (ultrasonicNudgeActive)
        Serial.println("right (ultrasonic)");
    else
        Serial.println("none");
}

void updateNudging()
{
    static unsigned long lastCheckMs = 0;
    static bool haveChecked = false;
    static bool checkRight = true;
    unsigned long now = millis();

    // End short IR pulses promptly, without waiting for the next 100 ms slot.
    if (irNudgeActive && now - irNudgeStartedMs >= IR_NUDGE_MS)
    {
        irNudgeActive = false;
        irNudgeEndedMs = now;
    }
    if (ultrasonicNudgeActive && now - ultrasonicNudgeStartedMs >= ULTRASONIC_NUDGE_MS)
    {
        ultrasonicNudgeActive = false;
        ultrasonicNudgeEndedMs = now;
    }

    if (haveChecked && now - lastCheckMs < NUDGE_INTERVAL_MS)
        return;
    lastCheckMs = now;
    haveChecked = true;

    // Sample ultrasonic distance on every check so close-range priority responds promptly.
    currentNudge = 0;
    double distanceCm = nudgeRight();
    bool ultrasonicPriority = distanceCm > 0
        && distanceCm <= ULTRASONIC_PRIORITY_DISTANCE_CM;

    int irReading = -1; // Stays -1 on checks that skip the IR sensor.
    if (ultrasonicPriority)
    {
        irNudgeActive = false;
        irNudgeEndedMs = now;
    }
    else if (!checkRight)
    {
        currentNudge = 0;
        irReading = nudgeLeft();
    }
    checkRight = !checkRight;

    if (DEBUG_MODE && DEBUG_NUDGING)
        printNudgeDebug(distanceCm, irReading);
}

bool isUltrasonicNudgeActive()
{
    return ultrasonicNudgeActive;
}

int printIRReadings()
{
    shineIR();
    delay(IR_SETTLE_MS);
    int illuminated = analogRead(IR_SENSOR_PIN);

    if (DEBUG_MODE)
    {
        Serial.print("IR on: ");
        Serial.println(illuminated);
    }

    return illuminated;
}
