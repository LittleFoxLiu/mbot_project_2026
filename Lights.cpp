#include "Lights.h"
#include "RobotConfig.h"

void setupLights()
{
    pinMode(LDR_PIN, INPUT);
    pinMode(IR_SENSOR_PIN, INPUT);

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
