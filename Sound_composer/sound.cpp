#include "sound.h"
#include <math.h>

// =====================================
// Adjustable Sound Parameters
// =====================================

namespace {

    constexpr float CROSSFEED = 0.40f;

    constexpr float NOTE_AMPLITUDE = 0.80f;
    constexpr float VOICE_GAIN = 0.18f;
    constexpr float MASTER_VOLUME = 0.85f;

    constexpr float DRY_GAIN = 0.75f;
    constexpr float WET_GAIN = 0.25f;

    constexpr float ATTACK_MS = 8.0f;
    constexpr float HOLD_MS = 5.0f;
    constexpr float DECAY_MS = 1200.0f;

    constexpr float PITCH_SWEEP_MS = 80.0f;

    // Bass parameters

    constexpr float BASS_AMPLITUDE = 0.50f;
    constexpr float BASS_GAIN = 0.10f;

    constexpr float BASS_ATTACK_MS = 90.0f;
    constexpr float BASS_HOLD_MS = 30.0f;
    constexpr float BASS_DECAY_MS = 1000.0f;
    constexpr float BASS_RELEASE_MS = 300.0f;

    constexpr unsigned long BASS_MIN_INTERVAL = 2800;
    constexpr unsigned long BASS_MAX_INTERVAL = 6000;

    int16_t customWaveform[256];

    void generateWaveform() {

        for (int i = 0; i < 256; i++) {

            float phase = 2.0f * PI * i / 256.0f;

            float sample =
                  1.00f * sinf(phase)
                + 0.20f * sinf(2.0f * phase)
                + 0.10f * sinf(3.0f * phase)
                + 0.04f * sinf(4.0f * phase)
                + 0.02f * sinf(5.0f * phase);

            sample /= 1.36f;

            customWaveform[i] =
                (int16_t)(sample * 32767.0f);
        }
    }
}


// =====================================
// Constructor: Audio Signal Connections
// =====================================

PiezoSoundEngine::PiezoSoundEngine()

    // Oscillators to envelopes

    : patch1(osc1, env1),
      patch2(osc2, env2),
      patch3(osc3, env3),
      patch4(osc4, env4),
      patch5(osc5, env5),
      patch6(osc6, env6),
      patch7(osc7, env7),
      patch8(osc8, env8),

      // Primary speaker 1: piezos 1–4

      patch9(env1, 0, mixerA, 0),
      patch10(env2, 0, mixerA, 1),
      patch11(env3, 0, mixerA, 2),
      patch12(env4, 0, mixerA, 3),

      // Primary speaker 2: piezos 5–8

      patch13(env5, 0, mixerB, 0),
      patch14(env6, 0, mixerB, 1),
      patch15(env7, 0, mixerB, 2),
      patch16(env8, 0, mixerB, 3),

      // Primary signals

      patch17(mixerA, 0, leftMixer, 0),
      patch18(mixerB, 0, rightMixer, 0),

      // Crossfeed: boundary between 4 and 5

      patch19(env4, 0, rightMixer, 1),
      patch20(env5, 0, leftMixer, 1),

      // Crossfeed: boundary between 8 and 1

      patch21(env1, 0, rightMixer, 2),
      patch22(env8, 0, leftMixer, 2),

      // Speaker 1 independent reverb

      patch23(leftMixer, filter),
      patch24(filter, reverb),

      // Speaker 2 independent reverb

      patch25(rightMixer, filter2),
      patch26(filter2, reverb2),

      // Speaker 1 dry and wet signals

      patch27(leftMixer, 0, finalMixer, 0),
      patch28(reverb, 0, finalMixer, 1),

      // Speaker 2 dry and wet signals

      patch29(rightMixer, 0, finalMixer2, 0),
      patch30(reverb2, 0, finalMixer2, 1),

      // Speaker 1: I2S1
      // Pins 7, 20, 21

      patch31(finalMixer, 0, output, 0),
      patch32(finalMixer, 0, output, 1),

      // Speaker 2: I2S2
      // Pins 2, 3, 4

      patch33(finalMixer2, 0, output2, 0),
      patch34(finalMixer2, 0, output2, 1),

      patch35(bassOsc, bassEnv),

// Send bass to both speakers
patch36(bassEnv, 0, leftMixer, 3),
patch37(bassEnv, 0, rightMixer, 3)

{}


// =====================================
// Initialize Sound Engine
// =====================================

void PiezoSoundEngine::begin() {

    AudioMemory(120);

    generateWaveform();

    osc1.arbitraryWaveform(customWaveform, 172.0f);
    osc2.arbitraryWaveform(customWaveform, 172.0f);
    osc3.arbitraryWaveform(customWaveform, 172.0f);
    osc4.arbitraryWaveform(customWaveform, 172.0f);
    osc5.arbitraryWaveform(customWaveform, 172.0f);
    osc6.arbitraryWaveform(customWaveform, 172.0f);
    osc7.arbitraryWaveform(customWaveform, 172.0f);
    osc8.arbitraryWaveform(customWaveform, 172.0f);


    // =================================
    // Oscillator initialization
    // =================================

    osc1.begin(
        NOTE_AMPLITUDE,
        frequencies[0],
        WAVEFORM_TRIANGLE
    );

    osc2.begin(
        NOTE_AMPLITUDE,
        frequencies[1],
        WAVEFORM_TRIANGLE
    );

    osc3.begin(
        NOTE_AMPLITUDE,
        frequencies[2],
        WAVEFORM_SINE
    );

    osc4.begin(
        NOTE_AMPLITUDE,
        frequencies[3],
        WAVEFORM_ARBITRARY
    );

    osc5.begin(
        NOTE_AMPLITUDE,
        frequencies[4],
        WAVEFORM_ARBITRARY
    );

    osc6.begin(
        NOTE_AMPLITUDE,
        frequencies[5],
        WAVEFORM_SINE
    );

    osc7.begin(
        NOTE_AMPLITUDE,
        frequencies[6],
        WAVEFORM_SINE
    );

    osc8.begin(
        NOTE_AMPLITUDE,
        frequencies[7],
        WAVEFORM_SINE
    );


    // =================================
    // Envelope initialization
    // =================================

    configureEnvelope(env1);
    configureEnvelope(env2);
    configureEnvelope(env3);
    configureEnvelope(env4);
    configureEnvelope(env5);
    configureEnvelope(env6);
    configureEnvelope(env7);
    configureEnvelope(env8);

    // Bass oscillator

bassOsc.begin(
    BASS_AMPLITUDE,
    164.81f,
    WAVEFORM_TRIANGLE
);

// Bass envelope

bassEnv.delay(0);
bassEnv.attack(BASS_ATTACK_MS);
bassEnv.hold(BASS_HOLD_MS);
bassEnv.decay(BASS_DECAY_MS);
bassEnv.sustain(0.0f);
bassEnv.release(BASS_RELEASE_MS);
bassEnv.releaseNoteOn(30.0f);

// Initialize bass timer

lastBassTime = millis();
bassInterval = 4000;


    // =================================
    // Primary voice gains
    // =================================

    for (int i = 0; i < 4; i++) {

        mixerA.gain(i, VOICE_GAIN);
        mixerB.gain(i, VOICE_GAIN);
    }

    // Reduce primary gain for boundary piezos

    // Piezo 1
    mixerA.gain(0, VOICE_GAIN * (1.0f - CROSSFEED));

    // Piezo 4
    mixerA.gain(3, VOICE_GAIN * (1.0f - CROSSFEED));

    // Piezo 5
    mixerB.gain(0, VOICE_GAIN * (1.0f - CROSSFEED));

    // Piezo 8
    mixerB.gain(3, VOICE_GAIN * (1.0f - CROSSFEED));


    // =================================
    // Spatial mixing
    // =================================

    // Primary signals

    leftMixer.gain(0, 1.0f);
    rightMixer.gain(0, 1.0f);

    // Boundary between piezos 4 and 5

    leftMixer.gain(1, VOICE_GAIN * CROSSFEED);
    rightMixer.gain(1, VOICE_GAIN * CROSSFEED);

    // Boundary between piezos 8 and 1

    leftMixer.gain(2, VOICE_GAIN * CROSSFEED);
    rightMixer.gain(2, VOICE_GAIN * CROSSFEED);

    // Unused mixer inputs

    leftMixer.gain(3, BASS_GAIN);
    rightMixer.gain(3, BASS_GAIN);

    // =================================
    // Independent low-pass filters
    // =================================

    setFilter(3500.0f, 0.707f);


    // =================================
    // Independent reverb
    // =================================

    setReverb(0.70f, 0.55f);


    // =================================
    // Final dry / wet mixing
    // =================================

    setMasterVolume(MASTER_VOLUME);

    finalMixer.gain(2, 0.0f);
    finalMixer.gain(3, 0.0f);

    finalMixer2.gain(2, 0.0f);
    finalMixer2.gain(3, 0.0f);


    // =================================
    // Pitch modulation initialization
    // =================================

    pitchDepth = 0.0f;

    for (int i = 0; i < NUM_SENSORS; i++) {

        startFreq[i] = frequencies[i];

        triggerTime[i] = 0;

        pitchActive[i] = false;
    }
}


// =====================================
// Piano-style Resonant Envelope
// =====================================

void PiezoSoundEngine::configureEnvelope(
    AudioEffectEnvelope &env) {

    env.delay(0);

    env.attack(ATTACK_MS);

    env.hold(HOLD_MS);

    env.decay(DECAY_MS);

    env.sustain(0.0f);

    env.release(200.0f);

    env.releaseNoteOn(10.0f);
}


// =====================================
// Trigger Sound
// =====================================

void PiezoSoundEngine::trigger(
    uint8_t sensorIndex) {

    if (sensorIndex >= NUM_SENSORS) return;

    setOscillator(
        sensorIndex,
        frequencies[sensorIndex],
        NOTE_AMPLITUDE
    );

    startFreq[sensorIndex] =
        frequencies[sensorIndex];

    triggerTime[sensorIndex] = millis();

    pitchActive[sensorIndex] =
        pitchDepth > 0.0f;

    retriggerEnvelope(sensorIndex);
}


// =====================================
// Set Oscillator
// =====================================

void PiezoSoundEngine::setOscillator(
    uint8_t index,
    float frequency,
    float amplitude) {

    switch (index) {

        case 0:
            osc1.frequency(frequency);
            osc1.amplitude(amplitude);
            break;

        case 1:
            osc2.frequency(frequency);
            osc2.amplitude(amplitude);
            break;

        case 2:
            osc3.frequency(frequency);
            osc3.amplitude(amplitude);
            break;

        case 3:
            osc4.frequency(frequency);
            osc4.amplitude(amplitude);
            break;

        case 4:
            osc5.frequency(frequency);
            osc5.amplitude(amplitude);
            break;

        case 5:
            osc6.frequency(frequency);
            osc6.amplitude(amplitude);
            break;

        case 6:
            osc7.frequency(frequency);
            osc7.amplitude(amplitude);
            break;

        case 7:
            osc8.frequency(frequency);
            osc8.amplitude(amplitude);
            break;
    }
}


// =====================================
// Envelope Trigger
// =====================================

void PiezoSoundEngine::retriggerEnvelope(
    uint8_t index) {

    switch (index) {

        case 0: env1.noteOn(); break;
        case 1: env2.noteOn(); break;
        case 2: env3.noteOn(); break;
        case 3: env4.noteOn(); break;
        case 4: env5.noteOn(); break;
        case 5: env6.noteOn(); break;
        case 6: env7.noteOn(); break;
        case 7: env8.noteOn(); break;
    }
}


// =====================================
// Real-time Pitch Modulation
// =====================================

void PiezoSoundEngine::update() {

    updateBass();

    unsigned long now = millis();

    for (int i = 0; i < NUM_SENSORS; i++) {

        if (!pitchActive[i]) continue;

        float elapsed =
            (float)(now - triggerTime[i]);

        float t = elapsed / PITCH_SWEEP_MS;

        if (t >= 1.0f) {

            setOscillator(
                i,
                frequencies[i],
                NOTE_AMPLITUDE
            );

            pitchActive[i] = false;

            continue;
        }

        float modulation =
            1.0f + pitchDepth * (1.0f - t);

        float currentFreq =
            startFreq[i] * modulation;

        setOscillator(
            i,
            currentFreq,
            NOTE_AMPLITUDE
        );
    }
}


// =====================================
// Global Reverb Control
// =====================================

void PiezoSoundEngine::setReverb(
    float roomSize,
    float damping) {

    roomSize = constrain(roomSize, 0.0f, 1.0f);
    damping = constrain(damping, 0.0f, 1.0f);

    // Speaker 1

    reverb.roomsize(roomSize);
    reverb.damping(damping);

    // Speaker 2

    reverb2.roomsize(roomSize);
    reverb2.damping(damping);
}


// =====================================
// Global Filter Control
// =====================================

void PiezoSoundEngine::setFilter(
    float cutoff,
    float resonance) {

    cutoff = constrain(cutoff, 100.0f, 15000.0f);
    resonance = constrain(resonance, 0.5f, 5.0f);

    // Speaker 1

    filter.setLowpass(0, cutoff, resonance);

    // Speaker 2

    filter2.setLowpass(0, cutoff, resonance);
}


// =====================================
// Pitch Modulation Control
// =====================================

void PiezoSoundEngine::setModulation(
    float depth) {

    pitchDepth =
        constrain(depth, 0.0f, 0.8f);
}


// =====================================
// Global Master Volume
// =====================================

void PiezoSoundEngine::setMasterVolume(
    float volume) {

    masterVolume =
        constrain(volume, 0.0f, 1.0f);

    // Speaker 1

    finalMixer.gain(
        0, DRY_GAIN * masterVolume
    );

    finalMixer.gain(
        1, WET_GAIN * masterVolume
    );

    // Speaker 2

    finalMixer2.gain(
        0, DRY_GAIN * masterVolume
    );

    finalMixer2.gain(
        1, WET_GAIN * masterVolume
    );
}

void PiezoSoundEngine::setTemperature(
    float temperature) {

    if (!isfinite(temperature)) return;

    temperatureC = constrain(
        temperature,
        0.0f,
        40.0f
    );
}
float PiezoSoundEngine::getBassFrequency() {

    if (temperatureC < 8.0f) {
        return 110.00f;      // A2
    }

    if (temperatureC < 16.0f) {
        return 138.59f;      // C#3
    }

    if (temperatureC < 24.0f) {
        return 164.81f;      // E3
    }

    if (temperatureC < 32.0f) {
        return 220.00f;      // A3
    }

    return 277.18f;          // C#4
}

void PiezoSoundEngine::triggerBass() {

    float frequency = getBassFrequency();

    // Slight variation in amplitude
    // Makes the bass sound less mechanical

    float amplitudeVariation =
        random(75, 101) / 100.0f;

    bassOsc.frequency(frequency);

    bassOsc.amplitude(
        BASS_AMPLITUDE * amplitudeVariation
    );

    bassEnv.noteOn();
}

void PiezoSoundEngine::updateBass() {

    unsigned long now = millis();

    if (now - lastBassTime < bassInterval) {
        return;
    }

    lastBassTime = now;

    // Higher temperature = shorter interval

    float temperatureRatio =
        temperatureC / 40.0f;

    float baseInterval =
        BASS_MAX_INTERVAL -
        temperatureRatio *
        (BASS_MAX_INTERVAL - BASS_MIN_INTERVAL);

    // Random timing variation: +/- 25%

    float timingVariation =
        random(75, 126) / 100.0f;

    bassInterval =
        (unsigned long)(
            baseInterval * timingVariation
        );

    // Occasionally skip a bass note
    // Keeps the accompaniment subtle

    if (random(0, 100) < 25) {
        return;
    }

    triggerBass();
}