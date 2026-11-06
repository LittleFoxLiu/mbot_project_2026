#include "Lights.h"
#include "RobotConfig.h"

static const uint8_t DECODER_A_PIN = 11; // Port 1, S1.
static const uint8_t DECODER_B_PIN = 12; // Port 1, S2.

void setupLights()
{
    // Start with red selected.
    digitalWrite(DECODER_A_PIN, HIGH);
    digitalWrite(DECODER_B_PIN, HIGH);
    pinMode(DECODER_A_PIN, OUTPUT);
    pinMode(DECODER_B_PIN, OUTPUT);
}

void shineIR()
{
    digitalWrite(DECODER_A_PIN, LOW);
    digitalWrite(DECODER_B_PIN, LOW);
}

void shineRed()
{
    digitalWrite(DECODER_A_PIN, HIGH);
    digitalWrite(DECODER_B_PIN, HIGH);
}

void shineGreen()
{
    digitalWrite(DECODER_A_PIN, HIGH);
    digitalWrite(DECODER_B_PIN, LOW);
}

void shineBlue()
{
    digitalWrite(DECODER_A_PIN, LOW);
    digitalWrite(DECODER_B_PIN, HIGH);
}
