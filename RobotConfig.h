#pragma once
#include <Arduino.h>

// Adjustable robot parameters.
const bool ENABLE_BLACK_LINE_STOP = true; // Stop at black lines, read the colour and turn.
const unsigned long COLOUR_RETRY_MS = 200; // Pause before re-reading an unknown colour.
const unsigned long WAYPOINT_REARM_MS = 500; // After a manoeuvre, the line must stay clear this long before the next waypoint counts.
const bool BLACK_LINE_REQUIRE_BOTH = true; // Less sensitive: both detectors must see black.
const double ULTRASONIC_PRIORITY_DISTANCE_CM = 8.3; // Use ultrasonic correction only within this distance; keep above the nudge distance.
const double ULTRASONIC_RIGHT_NUDGE_DISTANCE_CM = 7.8; // Turn right when the left-side distance is below this value.
const unsigned int ULTRASONIC_NUDGE_ANGLE_DEG = 10; // Fixed right-turn angle for an ultrasonic nudge.
const unsigned long ULTRASONIC_NUDGE_PAUSE_MS = 250; // Pause between fixed-angle right nudges.
const int FORWARD_SPEED = 255; // Base motor speed (0 to 255).
const unsigned long NUDGE_INTERVAL_MS = 100; // Sample ultrasonic every 100 ms and IR every 200 ms.
const bool DEBUG_NUDGING = true; // Print ultrasonic distance, IR drop and the active nudge on every check.
const bool COLOUR_SENSOR_READOUT_MODE = false; // true: print raw R/G/B readings while stopped; false: normal operation.

const int TURN_SPEED = 255;
const unsigned long STOP_SLOWDOWN_MS = 300; // Gradually slow down over the final 300 ms.
const int STOP_END_SPEED = 80; // Speed just before stopping; raise if the wheels stall.
static_assert(STOP_END_SPEED > 0 && STOP_END_SPEED <= 255, "STOP_END_SPEED must be 1 to 255");
const unsigned int CELEBRATION_BPM = 132; // Higher = faster music.
static_assert(CELEBRATION_BPM >= 30 && CELEBRATION_BPM <= 300,
              "CELEBRATION_BPM must be 30 to 300");

static_assert(FORWARD_SPEED > 0 && FORWARD_SPEED <= 255, "FORWARD_SPEED must be 1 to 255");
static_assert(TURN_SPEED > 0 && TURN_SPEED <= 255, "TURN_SPEED must be 1 to 255");

// Duration = calibrated speed * time / current speed, rounded to the nearest ms.
// UL keeps the products from overflowing Arduino's 16-bit int.
const unsigned long TURN_90_MS = (850UL * 100 + TURN_SPEED / 2) / TURN_SPEED;
// Single-wheel pivot estimate, reduced slightly to keep the U-turn under 180 degrees.
const unsigned long SINGLE_WHEEL_TURN_90_MS = (TURN_90_MS * 190UL + 50UL) / 100UL;
const unsigned long ULTRASONIC_NUDGE_MS = (TURN_90_MS * ULTRASONIC_NUDGE_ANGLE_DEG + 45) / 90;
const unsigned long ONE_GRID_TIME_MS = (1400UL * 150 + FORWARD_SPEED / 2) / FORWARD_SPEED;
static_assert(WAYPOINT_REARM_MS < ONE_GRID_TIME_MS,
              "WAYPOINT_REARM_MS must be shorter than one grid, or the next waypoint can be missed");

const unsigned long IR_SETTLE_MS = 50; // Initial IR settling time and interval between readings.
const int IR_DROP_THRESHOLD = 75; // Nudge when the reading drops by at least this much (lower = more sensitive); -1 disables nudging.
const unsigned long IR_NUDGE_MS = 50; // Stop the left wheel this long per IR nudge; longer = stronger left turn.
const unsigned long IR_NUDGE_PAUSE_MS = 250; // Pause between IR nudges.

const unsigned long COLOUR_SETTLE_MS = 500; // Hold each colour before reading the LDR.
const uint8_t COLOUR_SAMPLES = 8; // Average this many LDR reads per channel to reduce noise.
// Colour calibration: raw LDR readings on black and white paper.
const int RED_BLACK   = 573;
const int GREEN_BLACK = 525;
const int BLUE_BLACK  = 636;
const int RED_WHITE   = 957;
const int GREEN_WHITE = 928;
const int BLUE_WHITE  = 936;
const double MAX_COLOUR_DISTANCE = 0.2; // Readings further than this from every reference return unknown.

// Port 3: sensor inputs.
const uint8_t LDR_PIN       = A3; // S2
const uint8_t IR_SENSOR_PIN = A2; // S1

// Port 1: decoder controls. Port 4 is reserved for the line sensor.
const uint8_t DECODER_A_PIN = 11; // S1
const uint8_t DECODER_B_PIN = 12; // S2

// Decoder: G -> GND; Y0 -> IR, Y1 -> green, Y2 -> blue, Y3 -> red.
