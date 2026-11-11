#include "RobotConfig.h"
#include "Lights.h"
#include "Movement.h"
#include "Nudging.h"
#include "LineSensor.h"
#include "ColourSensor.h"
#include "Celebration.h"

// Initialize the robot hardware and optional diagnostic modes.
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

}

// Handle waypoints, refresh sensor nudges, and keep the robot moving.
void loop()
{
    handleWaypoint();

    updateNudging();
    if (DEBUG_MODE && DEBUG_NUDGING)
    {
        stopMotor();
    }
    else
    {
        moveForward();
    }

}
