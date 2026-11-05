#include "SoundAlgorithms.h"

#include <math.h>

#include "piezo.h"

namespace {
constexpr float SENSOR_ZERO = 1750.0f;
constexpr float IMPACT_THRESHOLD = 500.0f;
constexpr float REARM_THRESHOLD = 250.0f;
constexpr uint32_t IMPACT_COOLDOWN_MS = 400;
constexpr uint16_t NOTE_FREQUENCIES[] = {262, 330, 392};

uint32_t lastImpactTime = 0;
bool detectorArmed = true;
size_t lastNoteIndex = sizeof(NOTE_FREQUENCIES) / sizeof(NOTE_FREQUENCIES[0]);
}

void soundAlgorithmsBegin() {
	lastImpactTime = millis() - IMPACT_COOLDOWN_MS;
	detectorArmed = true;
	lastNoteIndex = sizeof(NOTE_FREQUENCIES) / sizeof(NOTE_FREQUENCIES[0]);
	randomSeed(analogRead(A0) ^ micros());
}

bool soundAlgorithmsUpdate(uint16_t& noteFrequency) {
	const int sample = piezoRead();
	const uint32_t now = millis();
	const float deviation = fabsf(static_cast<float>(sample) - SENSOR_ZERO);
	if (deviation <= REARM_THRESHOLD) {
		detectorArmed = true;
	}

	if (!detectorArmed || deviation <= IMPACT_THRESHOLD ||
		now - lastImpactTime < IMPACT_COOLDOWN_MS) {
		return false;
	}

	lastImpactTime = now;
	detectorArmed = false;
	const size_t noteCount = sizeof(NOTE_FREQUENCIES) /
						sizeof(NOTE_FREQUENCIES[0]);
	size_t noteIndex;
	do {
		noteIndex = random(0, noteCount);
	} while (noteIndex == lastNoteIndex);
	lastNoteIndex = noteIndex;
	noteFrequency = NOTE_FREQUENCIES[noteIndex];
	return true;
}