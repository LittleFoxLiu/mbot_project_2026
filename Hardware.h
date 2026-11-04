#pragma once
#include <MeDCMotor.h>
#include <MeUltrasonicSensor.h>
#include <MeLineFollower.h>
#include <MeBuzzer.h>

// Shared devices are defined once in Hardware.cpp.
extern MeDCMotor motor1;
extern MeDCMotor motor2;
extern MeUltrasonicSensor ultrasonic;
extern MeLineFollower lineSensor;
extern MeBuzzer buzzer;
