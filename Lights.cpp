#include "Lights.h"
#include "RobotConfig.h"

static const uint8_t DECODER_A_PIN = 11; // Port 1, S1.
static const uint8_t DECODER_B_PIN = 12; // Port 1, S2.

// Configure the decoder pins used to select colour LEDs and the IR emitter.
void setupLights()
{
    // Start with red selected.
    digitalWrite(DECODER_A_PIN, HIGH);
    digitalWrite(DECODER_B_PIN, HIGH);
    pinMode(DECODER_A_PIN, OUTPUT);
    pinMode(DECODER_B_PIN, OUTPUT);
}

// Select the IR emitter on the shared decoder.
void shineIR()
{
    digitalWrite(DECODER_A_PIN, LOW);
    digitalWrite(DECODER_B_PIN, LOW);
}

// Select the red LED on the shared decoder.
void shineRed()
{
    digitalWrite(DECODER_A_PIN, HIGH);
    digitalWrite(DECODER_B_PIN, HIGH);
}

// Select the green LED on the shared decoder.
void shineGreen()
{
    digitalWrite(DECODER_A_PIN, HIGH);
    digitalWrite(DECODER_B_PIN, LOW);
}

// Select the blue LED on the shared decoder.
void shineBlue()
{
    digitalWrite(DECODER_A_PIN, LOW);
    digitalWrite(DECODER_B_PIN, HIGH);
}
