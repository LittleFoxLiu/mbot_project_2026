#pragma once

enum ColourCode
{
    COLOUR_UNKNOWN = -1,
    COLOUR_RED ,         // Left
    COLOUR_GREEN,       // Right
    COLOUR_ORANGE,      // U-turn
    COLOUR_PINK,        // Two left turns
    COLOUR_LIGHT_BLUE,  // Two right turns
    COLOUR_WHITE        // Finish
};

int detectColour();
int readWaypointColour();
void actOnColour(int colour);
void runColourSensorReadoutMode(); // Print raw R/G/B readings continuously.
const char *colourName(int code);
