// MeMCore.h defines the board port table: include it in this source file only.
#include <MeMCore.h>
#include "Hardware.h"

MeDCMotor motor1(M1);
MeDCMotor motor2(M2);
MeUltrasonicSensor ultrasonic(PORT_2);
MeLineFollower lineSensor(PORT_4);
MeBuzzer buzzer;
