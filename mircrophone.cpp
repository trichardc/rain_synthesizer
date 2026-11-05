#include "Microphone.h"

// Receive the left I2S audio channel.
// INMP441 L/R is connected to GND.
AudioInputI2S i2sInput;

AudioRecordQueue micQueue;

AudioConnection micConnection(
    i2sInput, 0,
    micQueue, 0
);

void microphoneBegin() {
    AudioMemory(20);

    micQueue.begin();
}

bool microphoneAvailable() {
    return micQueue.available() > 0;
}

const int16_t* microphoneReadBlock() {
    return static_cast<const int16_t*>(
        micQueue.readBuffer()
    );
}

void microphoneReleaseBlock() {
    micQueue.freeBuffer();
}
