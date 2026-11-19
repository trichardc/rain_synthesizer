
#ifndef PIEZO_SOUND_ENGINE_H
#define PIEZO_SOUND_ENGINE_H

#include <Arduino.h>
#include <Audio.h>

class PiezoSoundEngine {

public:

    PiezoSoundEngine();

    // Initialize audio system
    void begin();

    // Call when a digital Piezo sensor generates an impulse
    // sensorIndex: 0 to 7
    void trigger(uint8_t sensorIndex);

    // Call continuously inside loop()
    void update();

    // Global sound controls
    void setReverb(float roomSize,
                   float damping);

    void setFilter(float cutoff,
                   float resonance);

    void setModulation(float depth);

    void setMasterVolume(float volume);
    void setTemperature(float temperature);


private:
// Temperature-dependent bass

float temperatureC = 20.0f;

unsigned long lastBassTime = 0;
unsigned long bassInterval = 4000;

void updateBass();
void triggerBass();
float getBassFrequency();

    // =====================================
// Oscillators
// =====================================

AudioSynthWaveform osc1;
AudioSynthWaveform osc2;
AudioSynthWaveform osc3;
AudioSynthWaveform osc4;
AudioSynthWaveform osc5;
AudioSynthWaveform osc6;
AudioSynthWaveform osc7;
AudioSynthWaveform osc8;

AudioSynthWaveform bassOsc;
// =====================================
// Envelopes
// =====================================

AudioEffectEnvelope env1;
AudioEffectEnvelope env2;
AudioEffectEnvelope env3;
AudioEffectEnvelope env4;
AudioEffectEnvelope env5;
AudioEffectEnvelope env6;
AudioEffectEnvelope env7;
AudioEffectEnvelope env8;

AudioEffectEnvelope bassEnv;

// =====================================
// Primary Mixers
// =====================================

AudioMixer4 mixerA;
AudioMixer4 mixerB;

// =====================================
// Spatial Mixers
// =====================================

AudioMixer4 leftMixer;
AudioMixer4 rightMixer;

// =====================================
// Independent Filters
// =====================================

AudioFilterBiquad filter;
AudioFilterBiquad filter2;

// =====================================
// Independent Reverb
// =====================================

AudioEffectFreeverb reverb;
AudioEffectFreeverb reverb2;

// =====================================
// Final Output Mixers
// =====================================

AudioMixer4 finalMixer;
AudioMixer4 finalMixer2;

// =====================================
// Independent I2S Outputs
// =====================================

AudioOutputI2S output;
AudioOutputI2S2 output2;

// =====================================
// Audio Connections
// =====================================

AudioConnection patch1;
AudioConnection patch2;
AudioConnection patch3;
AudioConnection patch4;
AudioConnection patch5;
AudioConnection patch6;
AudioConnection patch7;
AudioConnection patch8;
AudioConnection patch9;
AudioConnection patch10;
AudioConnection patch11;
AudioConnection patch12;
AudioConnection patch13;
AudioConnection patch14;
AudioConnection patch15;
AudioConnection patch16;
AudioConnection patch17;
AudioConnection patch18;
AudioConnection patch19;
AudioConnection patch20;
AudioConnection patch21;
AudioConnection patch22;
AudioConnection patch23;
AudioConnection patch24;
AudioConnection patch25;
AudioConnection patch26;
AudioConnection patch27;
AudioConnection patch28;
AudioConnection patch29;
AudioConnection patch30;
AudioConnection patch31;
AudioConnection patch32;
AudioConnection patch33;
AudioConnection patch34;
AudioConnection patch35;
AudioConnection patch36;
AudioConnection patch37;
    // Voice parameters
    static const uint8_t NUM_SENSORS = 8;

    float frequencies[NUM_SENSORS] = {
       110.00,  // A2
    164.81,  // E3
    220.00,  // A3
    277.18,  // C#4
    329.63,  // E4
    440.00,  // A4
    554.37,  // C#5
    659.25   // E5
    };

    float pitchDepth = 0.25f;

    float masterVolume = 0.95f;

    // Pitch modulation state
    float startFreq[NUM_SENSORS];
    unsigned long triggerTime[NUM_SENSORS];

    bool pitchActive[NUM_SENSORS];

    // Internal helpers
    void configureEnvelope(AudioEffectEnvelope &env);

    void setOscillator(uint8_t index,
                       float frequency,
                       float amplitude);

    void retriggerEnvelope(uint8_t index);

};

#endif
