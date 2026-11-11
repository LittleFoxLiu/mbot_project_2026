#pragma once
#include <Arduino.h>

// Adjustable robot parameters.
const unsigned long COLOUR_RETRY_MS = 200; // Pause before re-reading an unknown colour.
const double ULTRASONIC_PRIORITY_DISTANCE_CM = 8.8; // Use ultrasonic correction only within this distance; keep above the nudge distance.
const double ULTRASONIC_RIGHT_NUDGE_DISTANCE_CM = 7.8; // Turn right when the left-side distance is below this value.
const unsigned int ULTRASONIC_NUDGE_ANGLE_DEG = 10; // Fixed right-turn angle for an ultrasonic nudge.
const unsigned long ULTRASONIC_NUDGE_PAUSE_MS = 100; // Pause between fixed-angle right nudges.
const int FORWARD_SPEED = 255; // Base motor speed (0 to 255).
const unsigned long NUDGE_INTERVAL_MS = 100; // Sample ultrasonic every 100 ms and IR every 200 ms.
const unsigned long DEBUG_NUDGING_PRINT_INTERVAL_MS = 500; // Print IR/ultrasonic diagnostics every 0.5 seconds.
const bool DEBUG_MODE = false; // Enable diagnostic serial output and readout mode.
const bool DEBUG_NUDGING = false; // Print ultrasonic distance, raw IR reading and the active nudge on every check; stop motors.
const bool COLOUR_SENSOR_READOUT_MODE = false; // true: print raw R/G/B readings while stopped; false: normal operation.

const int TURN_SPEED = 255;
const unsigned int CELEBRATION_BPM = 132; // Higher = faster music.

const unsigned long IR_SETTLE_MS = 50; // Initial IR settling time and interval between readings.
const int IR_DROP_THRESHOLD = 65; // Nudge when the reading drops by at least this much (lower = more sensitive); -1 disables nudging.
const unsigned long IR_NUDGE_MS = 78; // Stop the left wheel this long per IR nudge; longer = stronger left turn.
const unsigned long IR_NUDGE_PAUSE_MS = 130; // Pause between IR nudges.

const unsigned long COLOUR_SETTLE_MS = 200; // Hold each colour before reading the LDR.
const uint8_t COLOUR_SAMPLES = 8; // Average this many LDR reads per channel to reduce noise.

// Raw LDR calibration readings on black and white paper (red, green, blue).
const int RED_BLACK = 504;
const int GREEN_BLACK = 560;
const int BLUE_BLACK = 711;
const int RED_WHITE = 952;
const int GREEN_WHITE = 939;
const int BLUE_WHITE = 956;

// Averaged raw readings from each test card: {colour code, red, green, blue}.
// Re-measure after hardware changes. Stored as a macro so the table is emitted
// only by ColourSensor.cpp, rather than copied into every file including config.
#define COLOUR_CALIBRATION_POINTS \
    {COLOUR_RED, 940, 820, 837}, \
    {COLOUR_RED, 942, 842, 849}, \
    {COLOUR_RED, 941, 834, 849}, \
    {COLOUR_GREEN, 872, 911, 912}, \
    {COLOUR_GREEN, 864, 899, 888}, \
    {COLOUR_GREEN, 862, 900, 888}, \
    {COLOUR_ORANGE, 946, 873, 862}, \
    {COLOUR_ORANGE, 945, 862, 852}, \
    {COLOUR_ORANGE, 946, 878, 863}, \
    {COLOUR_PINK, 947, 913, 923}, \
    {COLOUR_PINK, 952, 918, 931}, \
    {COLOUR_PINK, 949, 913, 926}, \
    {COLOUR_LIGHT_BLUE, 840, 885, 924}, \
    {COLOUR_LIGHT_BLUE, 856, 901, 945}, \
    {COLOUR_LIGHT_BLUE, 859, 900, 945}, \
    {COLOUR_WHITE, 950, 941, 950}, \
    {COLOUR_WHITE, 956, 949, 949}, \
    {COLOUR_WHITE, 956, 943, 950}
