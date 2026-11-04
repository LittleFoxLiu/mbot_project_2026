#include "RobotConfig.h"
#include "Lights.h"
#include "Movement.h"
#include "Nudging.h"
#include "LineSensor.h"
#include "ColourSensor.h"
#include "Celebration.h"

void setup()
{
    stopMotor();
    Serial.begin(9600);
    setupLights();
    setupIRReference();

    if (COLOUR_SENSOR_READOUT_MODE)
    {
        stopMotor();
        runColourSensorReadoutMode();
    }

    // celebrate(); // Play once on startup; press reset to replay.
}

void loop()
{
    // After a manoeuvre, black is ignored until the line has stayed clear for
    // WAYPOINT_REARM_MS, so one waypoint (or flicker at its edge) cannot trigger twice.
    static bool leavingWaypoint = false;
    static unsigned long lastBlackMs = 0;

    if (onBlackLine())
    {
        if (!leavingWaypoint)
        {
            stopMotor();
            actOnColour(readWaypointColour());
            leavingWaypoint = true;
        }
        lastBlackMs = millis(); // After a manoeuvre, the clear timer starts when it ends.
    }
    else if (millis() - lastBlackMs >= WAYPOINT_REARM_MS)
        leavingWaypoint = false;

    updateNudging();
    moveForward();

    // ir sensor debug
    //printIRReadings(); // Blocking diagnostics would delay the alternating checks.
}
