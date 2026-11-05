#include "speaker.h"

#include <Audio.h>

namespace {
constexpr uint8_t VOICE_COUNT = 6;
constexpr uint32_t RELEASE_MS = 300;
constexpr float VOICE_AMPLITUDE = 0.22f;

AudioSynthWaveformSine sine0;
AudioSynthWaveformSine sine1;
AudioSynthWaveformSine sine2;
AudioSynthWaveformSine sine3;
AudioSynthWaveformSine sine4;
AudioSynthWaveformSine sine5;
AudioEffectEnvelope envelope0;
AudioEffectEnvelope envelope1;
AudioEffectEnvelope envelope2;
AudioEffectEnvelope envelope3;
AudioEffectEnvelope envelope4;
AudioEffectEnvelope envelope5;
AudioMixer4 lowerVoiceMixer;
AudioMixer4 upperVoiceMixer;
AudioMixer4 outputMixer;
AudioOutputI2S i2sOutput;
AudioConnection sineToEnvelope0(sine0, 0, envelope0, 0);
AudioConnection sineToEnvelope1(sine1, 0, envelope1, 0);
AudioConnection sineToEnvelope2(sine2, 0, envelope2, 0);
AudioConnection sineToEnvelope3(sine3, 0, envelope3, 0);
AudioConnection sineToEnvelope4(sine4, 0, envelope4, 0);
AudioConnection sineToEnvelope5(sine5, 0, envelope5, 0);
AudioConnection envelopeToLowerMixer0(envelope0, 0, lowerVoiceMixer, 0);
AudioConnection envelopeToLowerMixer1(envelope1, 0, lowerVoiceMixer, 1);
AudioConnection envelopeToLowerMixer2(envelope2, 0, lowerVoiceMixer, 2);
AudioConnection envelopeToLowerMixer3(envelope3, 0, lowerVoiceMixer, 3);
AudioConnection envelopeToUpperMixer0(envelope4, 0, upperVoiceMixer, 0);
AudioConnection envelopeToUpperMixer1(envelope5, 0, upperVoiceMixer, 1);
AudioConnection lowerMixerToOutput(lowerVoiceMixer, 0, outputMixer, 0);
AudioConnection upperMixerToOutput(upperVoiceMixer, 0, outputMixer, 1);
AudioConnection outputToLeft(outputMixer, 0, i2sOutput, 0);
AudioConnection outputToRight(outputMixer, 0, i2sOutput, 1);

struct VoiceState {
    AudioSynthWaveformSine* oscillator;
    AudioEffectEnvelope* envelope;
    uint32_t releaseTime;
    uint32_t endTime;
    bool active;
    bool released;
};

VoiceState voices[VOICE_COUNT] = {
    {&sine0, &envelope0, 0, 0, false, false},
    {&sine1, &envelope1, 0, 0, false, false},
    {&sine2, &envelope2, 0, 0, false, false},
    {&sine3, &envelope3, 0, 0, false, false},
    {&sine4, &envelope4, 0, 0, false, false},
    {&sine5, &envelope5, 0, 0, false, false},
};
}

void speakerBegin() {
    AudioMemory(40);

    AudioSynthWaveformSine* oscillators[] = {
        &sine0, &sine1, &sine2, &sine3, &sine4, &sine5};
    AudioEffectEnvelope* envelopes[] = {
        &envelope0, &envelope1, &envelope2,
        &envelope3, &envelope4, &envelope5};

    for (uint8_t index = 0; index < VOICE_COUNT; ++index) {
        oscillators[index]->frequency(440.0f);
        oscillators[index]->amplitude(VOICE_AMPLITUDE);
        envelopes[index]->attack(8.0f);
        envelopes[index]->hold(0.0f);
        envelopes[index]->decay(0.0f);
        envelopes[index]->sustain(0.8f);
        envelopes[index]->release(static_cast<float>(RELEASE_MS));
        if (index < 4) {
            lowerVoiceMixer.gain(index, 0.5f);
        } else {
            upperVoiceMixer.gain(index - 4, 0.5f);
        }
    }
    outputMixer.gain(0, 0.7f);
    outputMixer.gain(1, 0.7f);
}

void speakerPlayNote(uint16_t frequency, uint32_t durationMs) {
    const uint32_t now = millis();
    uint8_t voiceIndex = VOICE_COUNT;
    for (uint8_t index = 0; index < VOICE_COUNT; ++index) {
        if (!voices[index].active ||
            static_cast<int32_t>(now - voices[index].endTime) >= 0) {
            voiceIndex = index;
            break;
        }
    }
    if (voiceIndex == VOICE_COUNT) {
        return;
    }

    const uint32_t releaseDelay = durationMs > RELEASE_MS
                                      ? durationMs - RELEASE_MS
                                      : 0;
    AudioNoInterrupts();
    voices[voiceIndex].oscillator->frequency(static_cast<float>(frequency));
    voices[voiceIndex].envelope->noteOn();
    AudioInterrupts();

    voices[voiceIndex].releaseTime = now + releaseDelay;
    voices[voiceIndex].endTime = now + durationMs;
    voices[voiceIndex].active = true;
    voices[voiceIndex].released = false;
}

void speakerUpdate() {
    const uint32_t now = millis();
    for (uint8_t index = 0; index < VOICE_COUNT; ++index) {
        if (!voices[index].active) {
            continue;
        }
        if (!voices[index].released &&
            static_cast<int32_t>(now - voices[index].releaseTime) >= 0) {
            AudioNoInterrupts();
            voices[index].envelope->noteOff();
            AudioInterrupts();
            voices[index].released = true;
        }
        if (static_cast<int32_t>(now - voices[index].endTime) >= 0) {
            voices[index].active = false;
        }
    }
}