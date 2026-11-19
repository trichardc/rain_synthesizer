
#include <Arduino.h>
#include <Audio.h>

// One sine oscillator
AudioSynthWaveformSine sine1;

// I2S output
AudioOutputI2S i2s1;

// Connect oscillator to both channels
AudioConnection patch1(sine1, 0, i2s1, 0);
AudioConnection patch2(sine1, 0, i2s1, 1);

void setup() {

    AudioMemory(20);

    sine1.frequency(440);

    // Start quietly
    sine1.amplitude(0.8);

}

void loop() {

}
