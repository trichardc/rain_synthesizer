#include "piezo.h"

namespace {
constexpr uint8_t PIEZO_PIN = 14;
}

void piezoBegin() {
    pinMode(PIEZO_PIN, INPUT);
    analogReadResolution(12);
}

int piezoRead() {
    return analogRead(PIEZO_PIN);
}