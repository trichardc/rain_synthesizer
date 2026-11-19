#pragma once

#include <Arduino.h>

static constexpr uint8_t PIEZO_COUNT = 8;

void piezoBegin();
bool piezoRead(uint8_t index);