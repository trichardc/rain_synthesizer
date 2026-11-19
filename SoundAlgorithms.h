#pragma once

#include <Arduino.h>

void soundAlgorithmsBegin();
bool soundAlgorithmsUpdate(uint16_t &noteFrequency);
bool soundAlgorithmsUpdate(uint16_t &noteFrequency, uint8_t &sensorIndex);