
#include <Arduino.h>
#include "piezo.h"
#include "SoundAlgorithms.h"
#include "Sound_composer/sound.h"

namespace {
PiezoSoundEngine soundEngine;
constexpr float MOCK_TEMPERATURE_C = 20.0f;
}

void setup() {
    Serial.begin(115200);
    piezoBegin();
    soundEngine.begin();
    soundEngine.setTemperature(MOCK_TEMPERATURE_C);
    soundAlgorithmsBegin();
}

void loop() {
    uint16_t noteFrequency;
    uint8_t sensorIndex;
    if (soundAlgorithmsUpdate(noteFrequency, sensorIndex)) {
        soundEngine.trigger(sensorIndex);
        Serial.print("Mock piezo ");
        Serial.print(sensorIndex);
        Serial.print(", note ");
        Serial.println(noteFrequency);
    }

    soundEngine.update();
}
