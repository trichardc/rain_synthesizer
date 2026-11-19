#include "piezo.h"

namespace {
// Set this to false to read real piezo inputs instead of synthetic pulses.
constexpr bool USE_SYNTHETIC_PIEZO_INPUT = true;

constexpr uint8_t PIEZO_PINS[PIEZO_COUNT] = {
    14, 15, 16, 17, 18, 19, 20, 21
};

constexpr uint32_t SYNTHETIC_INTERVAL_MS = 333;
constexpr uint32_t SYNTHETIC_PULSE_MS = 12;

bool syntheticHigh[PIEZO_COUNT] = {};
uint32_t syntheticPulseEnd[PIEZO_COUNT] = {};
uint32_t nextSyntheticPulseTime = 0;

void updateSyntheticPiezos(uint32_t now) {
    for (uint8_t index = 0; index < PIEZO_COUNT; ++index) {
        if (syntheticHigh[index] &&
            static_cast<int32_t>(now - syntheticPulseEnd[index]) >= 0) {
            syntheticHigh[index] = false;
        }
    }

    if (static_cast<int32_t>(now - nextSyntheticPulseTime) < 0) {
        return;
    }

    const uint8_t index = random(0, PIEZO_COUNT);
    syntheticHigh[index] = true;
    syntheticPulseEnd[index] = now + SYNTHETIC_PULSE_MS;

    nextSyntheticPulseTime = now + SYNTHETIC_INTERVAL_MS;
}
}

void piezoBegin() {
    randomSeed(static_cast<uint32_t>(micros()));

    if (!USE_SYNTHETIC_PIEZO_INPUT) {
        for (uint8_t i = 0; i < PIEZO_COUNT; ++i) {
            pinMode(PIEZO_PINS[i], INPUT);
        }
    }

    for (uint8_t i = 0; i < PIEZO_COUNT; ++i) {
        syntheticHigh[i] = false;
        syntheticPulseEnd[i] = 0;
    }

    nextSyntheticPulseTime = millis();
}

bool piezoRead(uint8_t index) {
    if (index >= PIEZO_COUNT) {
        return false;
    }

    if (USE_SYNTHETIC_PIEZO_INPUT) {
        updateSyntheticPiezos(millis());
        return syntheticHigh[index];
    }

    return digitalRead(PIEZO_PINS[index]) == HIGH;
}