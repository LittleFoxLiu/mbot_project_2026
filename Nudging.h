#pragma once

void setupIRReference();
void updateNudging(); // Call frequently; samples ultrasonic every 100 ms and checks IR every 200 ms.
int forwardNudge(int speed);
bool isIRNudgeActive();
bool isUltrasonicNudgeActive();
int printIRReadings(); // Blocking diagnostic; do not use while driving.
