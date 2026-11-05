#pragma once

#include <Arduino.h>

void speakerBegin();
void speakerPlayNote(uint16_t frequency, uint32_t durationMs);
void speakerUpdate();