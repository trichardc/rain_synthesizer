#pragma once
#include <cstdint>
#include <random>

// --- CONSTANTS ---
constexpr double PI = 3.14159265358979323846;

// --- PIEZOELECTRIC ---

// parameters
struct RainConfig
{
    double intensity = 0.30;         // 0 = no rain, 1 = heavy rain
    double dropMeanAmplitude = 0.20; // mean voltage bump per drop, normal dist.
    double decayTauSeconds = 0.25;   // exponential decay time of hit
    double baselineNoise = 0.005;
    double maxDropsPerSecond = 60.0; // for intensity = 1
};

class PiezoRainSensor
{

public:
    PiezoRainSensor(double sampleRateHz, RainConfig cfg = {}, uint32_t seed = 1234);
    double next(); // generates next data point, in [0, 1]
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

// --- TEMPERATURE ---
struct TempConfig
{
    double baselineCelsius = 20.0;
    double dailyAmplitude = 6.0; // half peak-to-peak
    double noiseSigma = 0.05;
    double arPhi = 0.9995; // random-walk drift
    double arSigma = 0.02;
    double startHour = 0.0;
};

class TemperatureSensor
{
public:
    TemperatureSensor(double sampleRateHz, TempConfig cfg = {}, uint32_t seed = 42);
    double next();

private:
    double dt_t, t_ = 0.0;
    TempConfig cfg_;
    std::mt19937 rng_;
    std::normal_distribution<double> noiseDist_, arDist_;
    double ar_ = 0.0;
};

// --- HUMIDITY ---
struct HumidityConfig
{
    double baselinePct = 55.0;
    double dailyAmplitude = 15.0;
    double noiseSigma = 0.3;
    double arPhi = 0.9999;
    double arSigma = 0.1;
};