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
    if (DEBUG_MODE)
        Serial.begin(9600);
    setupLights();
    setupColourSensor();
    setupIRReference();

    if (DEBUG_MODE && COLOUR_SENSOR_READOUT_MODE)
    {
        stopMotor();
        runColourSensorReadoutMode();
    }

    // celebrate(); // Play once on startup; press reset to replay.
}

void loop()
{
    handleWaypoint();

    updateNudging();
    moveForward();

    // ir sensor debug
    //printIRReadings(); // Blocking diagnostics would delay the alternating checks.
}
