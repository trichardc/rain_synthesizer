#pragma once
#include <cstdint>
#include <random>

constexpr double PI = 3.14159265358979323846;

// Portable clamp — works on C++11 and up (std::clamp needs C++17).
template <typename T>
inline T clamp_value(T v, T lo, T hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

struct RainConfig
{
    double intensity = 0.30;
    double dropMeanAmplitude = 0.20;
    double decayTauSeconds = 0.25;
    double baselineNoise = 0.005;
    double maxDropsPerSecond = 60.0;
};

class PiezoRainSensor
{
public:
    PiezoRainSensor(double sampleRateHz, RainConfig cfg = {}, uint32_t seed = 1234);
    double next();
    void setIntensity(double i);

private:
    double dt_;
    RainConfig cfg_;
    std::mt19937 rng_;
    std::exponential_distribution<double> ampDist_;
    std::normal_distribution<double> noiseDist_;
    double value_ = 0.0;
    double decayFactor_ = 0.0;
};

struct TempConfig
{
    double baselineCelsius = 20.0;
    double dailyAmplitude = 6.0;
    double noiseSigma = 0.05;
    double arPhi = 0.9995;
    double arSigma = 0.02;
    double startHour = 0.0;
};

class TemperatureSensor
{
public:
    TemperatureSensor(double sampleRateHz, TempConfig cfg = {}, uint32_t seed = 42);
    double next();

private:
    double dt_, t_ = 0.0;
    TempConfig cfg_;
    std::mt19937 rng_;
    std::normal_distribution<double> noiseDist_, arDist_;
    double ar_ = 0.0;
};

struct HumidityConfig
{
    double baselinePct = 55.0;
    double dailyAmplitude = 15.0;
    double noiseSigma = 0.3;
    double arPhi = 0.9999;
    double arSigma = 0.1;
};

class HumiditySensor
{
public:
    HumiditySensor(double sampleRateHz, HumidityConfig cfg = {}, uint32_t seed = 7);
    double next();

private:
    double dt_, t_ = 0.0;
    HumidityConfig cfg_;
    std::mt19937 rng_;
    std::normal_distribution<double> noiseDist_, arDist_;
    double ar_ = 0.0;
};

struct LightConfig
{
    double cloudiness = 0.30;
    double noiseSigma = 0.010;
    double cloudPhi = 0.9999;
    double cloudSigma = 0.002;
    double daylightStartHour = 6.0;
    double daylightEndHour = 18.0;
};

class LightSensor
{
public:
    LightSensor(double sampleRateHz, LightConfig cfg = {}, uint32_t seed = 99);
    double next();

private:
    double dt_, t_ = 0.0;
    LightConfig cfg_;
    std::mt19937 rng_;
    std::normal_distribution<double> noiseDist_, cloudDist_;
    double cloud_ = 0.0;
};

struct SensorSample
{
    double t;
    double piezo;
    double temperature;
    double humidity;
    double light;
};

class SensorStream
{
public:
    SensorStream(double sampleRateHz,
                 RainConfig rain = {},
                 TempConfig temp = {},
                 HumidityConfig hum = {},
                 LightConfig light = {},
                 uint32_t seed = 2024);

    SensorSample next();
    double sampleRate() const { return sampleRate_; }

    PiezoRainSensor &piezo() { return piezo_; }
    TemperatureSensor &temperature() { return temp_; }
    HumiditySensor &humidity() { return hum_; }
    LightSensor &light() { return light_; }

private:
    double sampleRate_, dt_, t_ = 0.0;
    PiezoRainSensor piezo_;
    TemperatureSensor temp_;
    HumiditySensor hum_;
    LightSensor light_;
};