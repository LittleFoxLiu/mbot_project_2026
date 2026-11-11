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
static bool waitingForIRCalibration = false;
static unsigned long irNudgeStartedMs = 0;
static unsigned long irNudgeEndedMs = 0;
static bool haveIRNudged = false;
static const unsigned long ULTRASONIC_NUDGE_MS =
    (((850UL * 100 + TURN_SPEED / 2) / TURN_SPEED) * ULTRASONIC_NUDGE_ANGLE_DEG + 45) / 90;

// Set up the IR sensor and capture a stable reference, with a live fallback if needed.
void setupIRReference()
{
    // Capture the IR-on reference at startup. If the sensor starts above 330,
    // use 290 temporarily and keep checking until a reading reaches 330 or less.
    // IR checks continue with the fallback while normal movement is underway.
    pinMode(IR_SENSOR_PIN, INPUT);
    shineIR();
    delay(IR_SETTLE_MS);
    int initialReading = analogRead(IR_SENSOR_PIN);
    if (initialReading > 330)
    {
        irStartupReading = 290;
        waitingForIRCalibration = true;
    }
    else
    {
        irStartupReading = initialReading;
    }

    if (DEBUG_MODE)
    {
        Serial.print("IR startup reference: ");
        Serial.println(irStartupReading);
        if (waitingForIRCalibration)
            Serial.println("IR reference is temporary; waiting for a reading <= 330");
    }
}

// Scale the selected steering correction to the current forward speed.
int forwardNudge(int speed)
{
    // updateNudging() selects only one sensor's correction at a time.
    // Reduce steering with the base speed during slowdown to avoid reversing a wheel.
    return (long)currentNudge * speed / FORWARD_SPEED;
}

// Report whether an IR-triggered left correction pulse is active.
bool isIRNudgeActive()
{
    return irNudgeActive;
}

// Compare the IR reading with its startup reference and start a timed left pulse when needed.
static int nudgeLeft()
{
    unsigned long now = millis();
    // The emitter stays on while driving; detectColour() switches back to it.
    int illuminated = analogRead(IR_SENSOR_PIN);

    if (waitingForIRCalibration && illuminated <= 330)
    {
        irStartupReading = illuminated;
        waitingForIRCalibration = false;
        if (DEBUG_MODE)
        {
            Serial.print("IR calibrated from live reading: ");
            Serial.println(irStartupReading);
        }
    }

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

// Read the ultrasonic sensor and start a timed right pulse when an obstacle is close.
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

// Print rate-limited sensor readings and the correction currently active.
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

// Expire correction pulses, sample sensors on schedule, and resolve close-wall priority.
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

    // Always sample ultrasonic; check IR on alternating slots unless close range has priority.
    currentNudge = 0;
    double distanceCm = nudgeRight();
    bool ultrasonicPriority = distanceCm > 0
        && distanceCm <= ULTRASONIC_PRIORITY_DISTANCE_CM;

    int irReading = -1; // Stays -1 on checks that skip the IR sensor.
    if (ultrasonicPriority)
    {
        // Close-range ultrasonic readings take precedence over IR steering.
        irNudgeActive = false;
        irNudgeEndedMs = now;
    }
    else if (!checkRight)
    {
        // IR is checked on every other slot to alternate sensor work.
        currentNudge = 0;
        irReading = nudgeLeft();
    }
    checkRight = !checkRight;

    if (DEBUG_MODE && DEBUG_NUDGING)
        printNudgeDebug(distanceCm, irReading);
}

// Report whether an ultrasonic-triggered right correction pulse is active.
bool isUltrasonicNudgeActive()
{
    return ultrasonicNudgeActive;
}

// Take and optionally print a settled IR reading for diagnostics while stopped.
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
