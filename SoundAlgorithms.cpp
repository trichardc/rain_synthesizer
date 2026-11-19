#include "SoundAlgorithms.h"

#include <math.h>

#include "piezo.h"

namespace {
constexpr uint32_t IMPACT_COOLDOWN_MS = 400;
constexpr uint16_t NOTE_FREQUENCIES[] = {262, 330, 392};

uint32_t lastImpactTime[PIEZO_COUNT] = {};
bool detectorArmed[PIEZO_COUNT] = {};
size_t lastNoteIndex = sizeof(NOTE_FREQUENCIES) / sizeof(NOTE_FREQUENCIES[0]);
}

void soundAlgorithmsBegin() {
	const uint32_t now = millis();
	for (uint8_t index = 0; index < PIEZO_COUNT; ++index) {
		lastImpactTime[index] = now - IMPACT_COOLDOWN_MS;
		detectorArmed[index] = true;
	}
	lastNoteIndex = sizeof(NOTE_FREQUENCIES) / sizeof(NOTE_FREQUENCIES[0]);
}

bool soundAlgorithmsUpdate(uint16_t &noteFrequency, uint8_t &sensorIndex) {
	const uint32_t now = millis();
	for (uint8_t index = 0; index < PIEZO_COUNT; ++index) {
		const bool signalHigh = piezoRead(index);
		if (!signalHigh) {
			detectorArmed[index] = true;
			continue;
		}
		if (!detectorArmed[index] ||
			now - lastImpactTime[index] < IMPACT_COOLDOWN_MS) {
			continue;
		}

		lastImpactTime[index] = now;
		detectorArmed[index] = false;
		sensorIndex = index;

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
	return false;
}

bool soundAlgorithmsUpdate(uint16_t &noteFrequency) {
	uint8_t sensorIndex = 0;
	return soundAlgorithmsUpdate(noteFrequency, sensorIndex);
}