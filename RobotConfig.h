#pragma once
#include <Arduino.h>

// Adjustable robot parameters.
const unsigned long COLOUR_RETRY_MS = 200; // Pause before re-reading an unknown colour.
const double ULTRASONIC_PRIORITY_DISTANCE_CM = 8.3; // Use ultrasonic correction only within this distance; keep above the nudge distance.
const double ULTRASONIC_RIGHT_NUDGE_DISTANCE_CM = 7.7; // Turn right when the left-side distance is below this value.
const unsigned int ULTRASONIC_NUDGE_ANGLE_DEG = 10; // Fixed right-turn angle for an ultrasonic nudge.
const unsigned long ULTRASONIC_NUDGE_PAUSE_MS = 250; // Pause between fixed-angle right nudges.
const int FORWARD_SPEED = 255; // Base motor speed (0 to 255).
const unsigned long NUDGE_INTERVAL_MS = 100; // Sample ultrasonic every 100 ms and IR every 200 ms.
const bool DEBUG_MODE = false; // Enable diagnostic serial output and readout mode.
const bool DEBUG_NUDGING = true; // Print ultrasonic distance, IR drop and the active nudge on every check.
const bool COLOUR_SENSOR_READOUT_MODE = true; // true: print raw R/G/B readings while stopped; false: normal operation.

const int TURN_SPEED = 255;
const unsigned int CELEBRATION_BPM = 132; // Higher = faster music.


const unsigned long IR_SETTLE_MS = 50; // Initial IR settling time and interval between readings.
const int IR_DROP_THRESHOLD = 72; // Nudge when the reading drops by at least this much (lower = more sensitive); -1 disables nudging.
const unsigned long IR_NUDGE_MS = 50; // Stop the left wheel this long per IR nudge; longer = stronger left turn.
const unsigned long IR_NUDGE_PAUSE_MS = 250; // Pause between IR nudges.

const unsigned long COLOUR_SETTLE_MS = 200; // Hold each colour before reading the LDR.
const uint8_t COLOUR_SAMPLES = 8; // Average this many LDR reads per channel to reduce noise.

// Raw LDR calibration readings on black and white paper (red, green, blue).
const int RED_BLACK = 605;
const int GREEN_BLACK = 610;
const int BLUE_BLACK = 720;
const int RED_WHITE = 963;
const int GREEN_WHITE = 947;
const int BLUE_WHITE = 960;

// Averaged raw readings from each test card: {colour code, red, green, blue}.
// Re-measure after hardware changes. Stored as a macro so the table is emitted
// only by ColourSensor.cpp, rather than copied into every file including config.
#define COLOUR_CALIBRATION_POINTS \
    {COLOUR_RED, 957, 846, 875}, \
    {COLOUR_RED, 958, 845, 874}, \
    {COLOUR_RED, 958, 845, 874}, \
    {COLOUR_GREEN, 879, 910, 887}, \
    {COLOUR_GREEN, 879, 901, 889}, \
    {COLOUR_GREEN, 879, 910, 887}, \
    {COLOUR_ORANGE, 964, 899, 896}, \
    {COLOUR_ORANGE, 964, 899, 896}, \
    {COLOUR_ORANGE, 964, 899, 896}, \
    {COLOUR_PINK, 959, 916, 930}, \
    {COLOUR_PINK, 959, 916, 930}, \
    {COLOUR_PINK, 959, 916, 930}, \
    {COLOUR_LIGHT_BLUE, 862, 890, 928}, \
    {COLOUR_LIGHT_BLUE, 862, 890, 928}, \
    {COLOUR_LIGHT_BLUE, 862, 890, 928}, \
    {COLOUR_WHITE, 960, 943, 951}, \
    {COLOUR_WHITE, 960, 943, 951}, \
    {COLOUR_WHITE, 960, 943, 951}
