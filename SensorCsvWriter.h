#pragma once
#include "SyntheticSensors.h"
#include <cstddef>
#include <fstream>
#include <string>
#include <vector>

// Writes SensorSample records to a CSV. Creates/appends to the file.
class SensorCsvWriter
{
public:
    explicit SensorCsvWriter(const std::string &path);

    void writeSample(const SensorSample &s);
    void writeBlock(const std::vector<SensorSample> &samples);
    void flush() { file_.flush(); }

private:
    void writeHeaderIfNeeded();

    std::ofstream file_;
    bool headerWritten_ = false;
};

// Convenience helper: pull `n` new samples from a stream and write them.
void writeNextBlock(SensorStream &stream, SensorCsvWriter &writer, std::size_t n);