#include <Arduino.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_VEML7700.h>
#include <Wire.h>
#include <math.h>

#include "Sound_composer/sound.h"

namespace {
constexpr uint8_t PIEZO_PINS[] = {14, 15, 16, 17, 18, 19};
constexpr uint8_t PIEZO_COUNT =
    sizeof(PIEZO_PINS) / sizeof(PIEZO_PINS[0]);
constexpr uint32_t PIEZO_RETRIGGER_MS = 100;
constexpr uint32_t ENVIRONMENT_INTERVAL_MS = 1000;

PiezoSoundEngine soundEngine;
Adafruit_SHT31 temperatureSensor;
Adafruit_VEML7700 lightSensor;

bool temperatureSensorReady = false;
bool lightSensorReady = false;
bool previousPiezoState[PIEZO_COUNT] = {};
uint32_t lastPiezoTrigger[PIEZO_COUNT] = {};
uint32_t lastEnvironmentRead = 0;

float temperatureC = NAN;
float humidityPercent = NAN;
float lightLux = NAN;

void readEnvironment() {
    if (temperatureSensorReady) {
        const float measuredTemperature = temperatureSensor.readTemperature();
        const float measuredHumidity = temperatureSensor.readHumidity();

        if (isfinite(measuredTemperature)) {
            temperatureC = measuredTemperature;
            soundEngine.setTemperature(temperatureC);
        }
        if (isfinite(measuredHumidity)) {
            humidityPercent = measuredHumidity;
        }
    }

    if (lightSensorReady) {
        const float measuredLux = lightSensor.readLux();
        if (isfinite(measuredLux)) {
            lightLux = measuredLux;
        }
    }
}

void printReading(const __FlashStringHelper *label, float value, uint8_t decimals) {
    Serial.print(label);
    if (isfinite(value)) {
        Serial.print(value, decimals);
    } else {
        Serial.print(F("N/A"));
    }
}

void pollPiezoInputs(uint32_t now) {
    for (uint8_t index = 0; index < PIEZO_COUNT; ++index) {
        const bool currentState = digitalRead(PIEZO_PINS[index]) == HIGH;
        const bool risingEdge = currentState && !previousPiezoState[index];
        previousPiezoState[index] = currentState;

        if (risingEdge &&
            now - lastPiezoTrigger[index] >= PIEZO_RETRIGGER_MS) {
            lastPiezoTrigger[index] = now;
            soundEngine.trigger(index);
            Serial.print(F("Piezo "));
            Serial.println(index);
        }
    }
}
}

void setup() {
    Serial.begin(115200);

    for (uint8_t index = 0; index < PIEZO_COUNT; ++index) {
        pinMode(PIEZO_PINS[index], INPUT);
        lastPiezoTrigger[index] = millis() - PIEZO_RETRIGGER_MS;
    }

    Wire.begin();
    Wire.setClock(100000);
    temperatureSensorReady = temperatureSensor.begin(0x44);
    lightSensorReady = lightSensor.begin();

    if (lightSensorReady) {
        lightSensor.setGain(VEML7700_GAIN_1);
        lightSensor.setIntegrationTime(VEML7700_IT_25MS);
    }

    Serial.println(temperatureSensorReady ? F("SHT31 OK") : F("SHT31 NOT FOUND"));
    Serial.println(lightSensorReady ? F("VEML7700 OK") : F("VEML7700 NOT FOUND"));

    soundEngine.begin();
    readEnvironment();
    lastEnvironmentRead = millis();
}

void loop() {
    const uint32_t now = millis();
    pollPiezoInputs(now);

    if (now - lastEnvironmentRead >= ENVIRONMENT_INTERVAL_MS) {
        readEnvironment();
        lastEnvironmentRead = now;

        printReading(F("Temp:"), temperatureC, 2);
        Serial.print(F(",Hum:"));
        if (isfinite(humidityPercent)) {
            Serial.print(humidityPercent, 2);
        } else {
            Serial.print(F("N/A"));
        }
        Serial.print(F(",Lux:"));
        if (isfinite(lightLux)) {
            Serial.println(lightLux, 1);
        } else {
            Serial.println(F("N/A"));
        }
    }

    soundEngine.update();
}
