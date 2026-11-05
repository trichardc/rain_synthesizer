
#pragma once

#include <Arduino.h>
#include <Audio.h>

constexpr size_t MIC_BLOCK_SIZE = 128;

void microphoneBegin();

bool microphoneAvailable();

const int16_t* microphoneReadBlock();

void microphoneReleaseBlock();
