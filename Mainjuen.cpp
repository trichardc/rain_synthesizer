#include <Arduino.h>
#include "piezo.h"
#include "SoundAlgorithms.h"
#include "speaker.h"

void setup() {
    Serial.begin(115200);
    delay(1000);

    piezoBegin();
    speakerBegin();
    soundAlgorithmsBegin();
    Serial.println("Piezo impact synthesizer ready");
}

void loop() {
    static uint32_t lastReadingTime = 0;
    const uint32_t now = millis();

    uint16_t noteFrequency;
    if (soundAlgorithmsUpdate(noteFrequency)) {
        speakerPlayNote(noteFrequency, 2000);
        Serial.print("Impact note: ");
        Serial.println(noteFrequency);
    }

    if (now - lastReadingTime >= 50) {
        Serial.print("Piezo: ");
        Serial.println(piezoRead());
        lastReadingTime = now;
    }

    speakerUpdate();
    delay(2);