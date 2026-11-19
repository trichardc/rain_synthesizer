#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_VEML7700.h>

// ===== Digital piezo pins (direct, no multiplexer) =====
const int PIEZO_PINS[] = {14, 15, 16, 17, 18, 19, 20, 21};

const int NUM_PIEZOS = sizeof(PIEZO_PINS) / sizeof(PIEZO_PINS[0]);

// ===== I2C Sensors =====
Adafruit_SHT31 sht31 = Adafruit_SHT31();
Adafruit_VEML7700 veml = Adafruit_VEML7700();
bool sht31_ok = false, veml_ok = false;

// ===== Scheduling =====
const unsigned long REPORT_MS = 33; // 30 Hz output
const unsigned long I2C_MS = 1000;  // env sensors once/sec

unsigned long lastReport = 0;
unsigned long lastI2C = 0;

// Per-channel peak tracking
int chMin[8]; // Sized for the eventual 8-channel case
int chMax[8];

float tempC = -999, humid = -999, lux = -999;

void setup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 3000)
        ;

    for (int i = 0; i < NUM_PIEZOS; i++)
    {
        pinMode(PIEZO_PINS[i], INPUT);
        chMin[i] = 1;
        chMax[i] = 0;
    }

    Wire.begin();
    Wire.setClock(100000);

    sht31_ok = sht31.begin(0x44);
    Serial.println(sht31_ok ? "SHT31 OK" : "SHT31 NOT FOUND");

    veml_ok = veml.begin();
    if (veml_ok)
    {
        veml.setGain(VEML7700_GAIN_1);
        veml.setIntegrationTime(VEML7700_IT_25MS);
        Serial.println("VEML7700 OK");
    }
    else
    {
        Serial.println("VEML7700 NOT FOUND");
    }

    Serial.println("direct piezo mode started");
}

void loop()
{
    // 1) Read every piezo pin back-to-back
    for (int ch = 0; ch < NUM_PIEZOS; ch++)
    {
        int v = digitalRead(PIEZO_PINS[ch]);
        if (v == LOW)
            chMin[ch] = 0;
        if (v == HIGH)
            chMax[ch] = 1;
    }

    unsigned long now = millis();

    // 2) Environmental sensors, once per second
    if (now - lastI2C >= I2C_MS)
    {
        lastI2C = now;
        if (sht31_ok)
        {
            float tt = sht31.readTemperature();
            float hh = sht31.readHumidity();
            if (!isnan(tt))
                tempC = tt;
            if (!isnan(hh))
                humid = hh;
        }
        if (veml_ok)
        {
            float ll = veml.readLux();
            if (!isnan(ll))
                lux = ll;
        }
    }

    // 3) Report at 30 Hz
    if (now - lastReport >= REPORT_MS)
    {
        lastReport = now;

        for (int ch = 0; ch < NUM_PIEZOS; ch++)
        {
            Serial.print("P");
            Serial.print(ch);
            Serial.print(":");
            Serial.print(chMax[ch] - chMin[ch]); // 0 = low, 1 = one pulse
            Serial.print(",");
        }
        Serial.print("Temp:");
        Serial.print(tempC, 2);
        Serial.print(",Hum:");
        Serial.print(humid, 2);
        Serial.print(",Lux:");
        Serial.println(lux, 1);

        // Reset windows for the next report
        for (int ch = 0; ch < NUM_PIEZOS; ch++)
        {
            chMin[ch] = 1;
            chMax[ch] = 0;
        }
    }
}