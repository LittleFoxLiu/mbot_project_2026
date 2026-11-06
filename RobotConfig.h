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
const bool DEBUG_MODE = true; // Enable diagnostic serial output and readout mode.
const bool DEBUG_NUDGING = true; // Print ultrasonic distance, raw IR reading and the active nudge on every check; stop motors.
const bool COLOUR_SENSOR_READOUT_MODE = false; // true: print raw R/G/B readings while stopped; false: normal operation.

const int TURN_SPEED = 255;
const unsigned int CELEBRATION_BPM = 132; // Higher = faster music.

const unsigned long IR_SETTLE_MS = 50; // Initial IR settling time and interval between readings.
const int IR_DROP_THRESHOLD = 73; // Nudge when the reading drops by at least this much (lower = more sensitive); -1 disables nudging.
const unsigned long IR_NUDGE_MS = 50; // Stop the left wheel this long per IR nudge; longer = stronger left turn.
const unsigned long IR_NUDGE_PAUSE_MS = 130; // Pause between IR nudges.

const unsigned long COLOUR_SETTLE_MS = 150; // Hold each colour before reading the LDR.
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
    {COLOUR_RED, 937, 792, 834}, \
    {COLOUR_RED, 939, 810, 845}, \
    {COLOUR_RED, 940, 821, 850}, \
    {COLOUR_GREEN, 840, 895, 887}, \
    {COLOUR_GREEN, 828, 892, 886}, \
    {COLOUR_GREEN, 832, 894, 888}, \
    {COLOUR_ORANGE, 944, 862, 855}, \
    {COLOUR_ORANGE, 944, 870, 865}, \
    {COLOUR_ORANGE, 943, 867, 862}, \
    {COLOUR_PINK, 940, 909, 932}, \
    {COLOUR_PINK, 945, 909, 935}, \
    {COLOUR_PINK, 939, 908, 932}, \
    {COLOUR_LIGHT_BLUE, 824, 888, 937}, \
    {COLOUR_LIGHT_BLUE, 833, 890, 939}, \
    {COLOUR_LIGHT_BLUE, 825, 887, 938}, \
    {COLOUR_WHITE, 946, 939, 954}, \
    {COLOUR_WHITE, 956, 951, 963}, \
    {COLOUR_WHITE, 947, 941, 956}
