#pragma once

void handleWaypoint(); // Detect and handle a waypoint, then wait for the line to clear before rearming.
void waitWithLineMonitoring(unsigned long durationMs); // Keep checking during sensor waits.
