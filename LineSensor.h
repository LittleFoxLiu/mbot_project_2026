#pragma once

void printBlackLineReadings();
bool onBlackLine(); // True while the line sensor sees black; does not touch the motors.
bool stopOnBlackLine(); // Stops motors only while black is detected; does not latch.
void waitWithLineMonitoring(unsigned long durationMs); // Keep checking during sensor waits.
