#include "Celebration.h"
#include "Hardware.h"
#include "Movement.h"
#include "RobotConfig.h"

static_assert(CELEBRATION_BPM >= 30 && CELEBRATION_BPM <= 300,
              "CELEBRATION_BPM must be 30 to 300");

static void playCelebrationNote(uint16_t frequency, uint8_t eighthNotes)
{
    unsigned long durationMs = 30000UL * eighthNotes / CELEBRATION_BPM;
    if (frequency == 0)
    {
        delay(durationMs); // Rest; never pass zero frequency to MeBuzzer::tone().
        return;
    }

    // MeBuzzer::tone() blocks for its duration. Leave a short gap between notes.
    unsigned long soundMs = durationMs * 9 / 10;
    buzzer.tone(frequency, soundMs);
    buzzer.noTone();
    delay(durationMs - soundMs);
}

void celebrate()
{
    stopMotor();
    buzzer.noTone();

    // Erika, Herms Niel: single melody in G major, without accompaniment.
    // Melody reference: https://en.wikipedia.org/wiki/Erika_(song)#Music
    // Each pair is {frequency in Hz, duration in eighth notes}; 0 Hz is a rest.
    // B4 C5 D5 G5 B5 A5 G5 ...; retain the three-beat rests between phrases.
    static const uint16_t verse[][2] = {
        {494, 3}, {523, 1}, {587, 2}, {587, 2}, {587, 2}, {784, 2},
        {784, 2}, {988, 2}, {988, 3}, {880, 1}, {784, 2}, {0, 6},
        {740, 2}, {784, 2}, {880, 2}, {0, 6},
        {988, 3}, {880, 1}, {784, 2}, {0, 6}
    };
    static const uint16_t bridge[][2] = {
        {587, 3}, {784, 1}, {740, 2}, {740, 2}, {740, 2}, {740, 2},
        {659, 2}, {740, 2}, {784, 2}, {0, 6},
        {740, 3}, {784, 1}, {880, 2}, {880, 2}, {880, 2}, {880, 2},
        {1175, 3}, {1047, 1}, {988, 2}, {0, 6}
    };

    // One verse: opening twice, middle section, then the opening once more.
    for (uint8_t section = 0; section < 4; section++)
    {
        if (section == 2)
        {
            for (unsigned int i = 0; i < sizeof(bridge) / sizeof(bridge[0]); i++)
                playCelebrationNote(bridge[i][0], bridge[i][1]);
        }
        else
        {
            for (unsigned int i = 0; i < sizeof(verse) / sizeof(verse[0]); i++)
                playCelebrationNote(verse[i][0], verse[i][1]);
        }
    }

    buzzer.noTone();
    stopMotor();
}
