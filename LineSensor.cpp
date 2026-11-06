#include "LineSensor.h"
#include "Hardware.h"
#include "Movement.h"
#include "RobotConfig.h"
#include "ColourSensor.h"

static const bool ENABLE_BLACK_LINE_STOP = true; // Stop at black lines, read the colour and turn.
static const bool BLACK_LINE_REQUIRE_BOTH = true; // Less sensitive: both detectors must see black.
static const unsigned long WAYPOINT_REARM_MS = 500;
static_assert(WAYPOINT_REARM_MS < ((1400UL * 150 + FORWARD_SPEED / 2) / FORWARD_SPEED),
              "Waypoint rearm must be shorter than one grid");
static bool leavingWaypoint = false;
static unsigned long lastBlackMs = 0;

static bool isBlackLine(uint8_t state)
{
    // Requiring both detectors rejects single-detector edge/noise readings.
    return BLACK_LINE_REQUIRE_BOTH ? state == S1_IN_S2_IN : state != S1_OUT_S2_OUT;
}

static bool onBlackLine()
{
    return ENABLE_BLACK_LINE_STOP && isBlackLine(lineSensor.readSensors());
}

static bool stopOnBlackLine()
{
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

void handleWaypoint()
{
    if (onBlackLine())
    {
        if (!leavingWaypoint)
        {
            stopMotor();
            actOnColour(readWaypointColour());
            leavingWaypoint = true;
        }
        lastBlackMs = millis();
    }
    else if (millis() - lastBlackMs >= WAYPOINT_REARM_MS)
        leavingWaypoint = false;
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
