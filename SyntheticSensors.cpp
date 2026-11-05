#include "SyntheticSensors.h"
#include <algorithm>
#include <cmath>

// -------------------------------------------------------------------------
// Piezo rain sensor
// -------------------------------------------------------------------------
PiezoRainSensor::PiezoRainSensor(double sampleRateHz, RainConfig cfg, uint32_t seed)
    : dt_(1.0 / sampleRateHz),
      cfg_(cfg),
      rng_(seed),
      ampDist_(1.0 / std::max(1e-9, cfg.dropMeanAmplitude)),
      noiseDist_(0.0, std::max(1e-9, cfg.baselineNoise))
{
    decayFactor_ = std::exp(-dt_ / std::max(1e-9, cfg_.decayTauSeconds));
}

double PiezoRainSensor::next()
{
    // 1) decay existing energy
    value_ *= decayFactor_;

    // 2) Poisson arrival of new raindrops during this sample
    const double rate = cfg_.maxDropsPerSecond * cfg_.intensity;
    const double lambda = rate * dt_;
    if (lambda > 0.0)
    {
        std::poisson_distribution<int> drops(lambda);
        const int k = drops(rng_);
        for (int i = 0; i < k; ++i)
            value_ += ampDist_(rng_);
    }

    // 3) baseline noise
    value_ += noiseDist_(rng_);

    return std::clamp(value_, 0.0, 1.0);
}

void PiezoRainSensor::setIntensity(double i)
{
    cfg_.intensity = std::clamp(i, 0.0, 1.0);
}

// -------------------------------------------------------------------------
// Temperature sensor
// -------------------------------------------------------------------------
TemperatureSensor::TemperatureSensor(double sampleRateHz, TempConfig cfg, uint32_t seed)
    : dt_(1.0 / sampleRateHz),
      t_(cfg.startHour * 3600.0),
      cfg_(cfg),
      rng_(seed),
      noiseDist_(0.0, std::max(1e-9, cfg.noiseSigma)),
      arDist_(0.0, std::max(1e-9, cfg.arSigma))
{
}

double TemperatureSensor::next()
{
    t_ += dt_;
    // Diurnal cycle: peaks ~15:00, troughs ~03:00.
    const double dayFrac = std::fmod(t_ / 86400.0, 1.0);
    const double daily = cfg_.dailyAmplitude * std::sin(2.0 * PI * (dayFrac - 0.375));
    // Slow AR(1) drift + white noise
    ar_ = cfg_.arPhi * ar_ + arDist_(rng_);
    return cfg_.baselineCelsius + daily + ar_ + noiseDist_(rng_);
}

// -------------------------------------------------------------------------
// Humidity sensor
// -------------------------------------------------------------------------
HumiditySensor::HumiditySensor(double sampleRateHz, HumidityConfig cfg, uint32_t seed)
    : dt_(1.0 / sampleRateHz),
      cfg_(cfg),
      rng_(seed),
      noiseDist_(0.0, std::max(1e-9, cfg.noiseSigma)),
      arDist_(0.0, std::max(1e-9, cfg.arSigma))
{
}

double HumiditySensor::next()
{
    t_ += dt_;
    // Anti-correlated with temperature (peaks before dawn)
    const double dayFrac = std::fmod(t_ / 86400.0, 1.0);
    const double daily = -cfg_.dailyAmplitude * std::sin(2.0 * PI * (dayFrac - 0.375));
    ar_ = cfg_.arPhi * ar_ + arDist_(rng_);
    return std::clamp(cfg_.baselinePct + daily + ar_ + noiseDist_(rng_), 0.0, 100.0);
}

// -------------------------------------------------------------------------
// Light sensor
// -------------------------------------------------------------------------
LightSensor::LightSensor(double sampleRateHz, LightConfig cfg, uint32_t seed)
    : dt_(1.0 / sampleRateHz),
      cfg_(cfg),
      rng_(seed),
      noiseDist_(0.0, std::max(1e-9, cfg.noiseSigma)),
      cloudDist_(0.0, std::max(1e-9, cfg.cloudSigma))
{
}

double LightSensor::next()
{
    t_ += dt_;
    const double hour = std::fmod(t_ / 3600.0, 24.0);

    // Half-sine daylight curve, zero outside the daylight window
    double base = 0.0;
    if (hour > cfg_.daylightStartHour && hour < cfg_.daylightEndHour)
    {
        const double frac = (hour - cfg_.daylightStartHour) /
                            (cfg_.daylightEndHour - cfg_.daylightStartHour);
        base = std::sin(PI * frac);
    }

    // Slowly drifting cloud cover
    cloud_ = cfg_.cloudPhi * cloud_ + cloudDist_(rng_);
    const double attenuation = std::clamp(cfg_.cloudiness + cloud_, 0.0, 1.0);

    const double v = base * (1.0 - attenuation) + noiseDist_(rng_);
    return std::clamp(v, 0.0, 1.0);
}

// -------------------------------------------------------------------------
// SensorStream
// -------------------------------------------------------------------------
SensorStream::SensorStream(double sampleRateHz,
                           RainConfig rain, TempConfig temp,
                           HumidityConfig hum, LightConfig light,
                           uint32_t seed)
    : sampleRate_(sampleRateHz),
      dt_(1.0 / sampleRateHz),
      piezo_(sampleRateHz, rain, seed ^ 0xA5A5A5A5u),
      temp_(sampleRateHz, temp, seed ^ 0x5A5A5A5Au),
      hum_(sampleRateHz, hum, seed ^ 0x12345678u),
      light_(sampleRateHz, light, seed ^ 0x9ABCDEF0u)
{
}

SensorSample SensorStream::next()
{
    SensorSample s;
    s.t = t_;
    s.piezo = piezo_.next();
    s.temperature = temp_.next();
    s.humidity = hum_.next();
    s.light = light_.next();
    t_ += dt_;
    return s;
}