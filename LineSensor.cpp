#include "LineSensor.h"
#include "Hardware.h"
#include "Movement.h"
#include "RobotConfig.h"

static bool isBlackLine(uint8_t state)
{
    // Requiring both detectors rejects single-detector edge/noise readings.
    return BLACK_LINE_REQUIRE_BOTH ? state == S1_IN_S2_IN : state != S1_OUT_S2_OUT;
}

void printBlackLineReadings()
{
    static unsigned long lastPrintMs = 0;
    static bool havePrinted = false;
    unsigned long now = millis();
    if (havePrinted && now - lastPrintMs < 200)
        return; // Limit serial traffic when the driving loop runs quickly.
    lastPrintMs = now;
    havePrinted = true;

    uint8_t state = lineSensor.readSensors();
    //Serial.println(isBlackLine(state) ? "Black line: YES" : "Black line: NO");
}

bool onBlackLine()
{
    return ENABLE_BLACK_LINE_STOP && isBlackLine(lineSensor.readSensors());
}

bool stopOnBlackLine()
{
    printBlackLineReadings();
    if (!ENABLE_BLACK_LINE_STOP)
        return false;

    // Read the current state every time; clearing the line permits movement again.
    if (isBlackLine(lineSensor.readSensors()))
    {
        stopMotor();
        return true;
    }
    return false;
}

void waitWithLineMonitoring(unsigned long durationMs)
{
    unsigned long startedMs = millis();
    while (millis() - startedMs < durationMs)
    {
        stopOnBlackLine(); // Keep the program running even if the motors are stopped.
        delay(1);
    }
}
