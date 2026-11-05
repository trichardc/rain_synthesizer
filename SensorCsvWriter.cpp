#include "SensorCsvWriter.h"
#include <iomanip>
#include <stdexcept>

SensorCsvWriter::SensorCsvWriter(const std::string &path)
    : file_(path)
{
    if (!file_.is_open())
        throw std::runtime_error("Cannot open CSV file: " + path);

    file_ << std::fixed << std::setprecision(6);
}

void SensorCsvWriter::writeHeaderIfNeeded()
{
    if (headerWritten_)
        return;
    file_ << "t,piezo,temperature,humidity,light\n";
    headerWritten_ = true;
}

void SensorCsvWriter::writeSample(const SensorSample &s)
{
    writeHeaderIfNeeded();
    file_ << s.t << ','
          << s.piezo << ','
          << s.temperature << ','
          << s.humidity << ','
          << s.light << '\n';
}

void SensorCsvWriter::writeBlock(const std::vector<SensorSample> &samples)
{
    for (const auto &s : samples)
        writeSample(s);
}

void writeNextBlock(SensorStream &stream, SensorCsvWriter &writer, std::size_t n)
{
    std::vector<SensorSample> block;
    block.reserve(n);
    for (std::size_t i = 0; i < n; ++i)
        block.push_back(stream.next());
    writer.writeBlock(block);
}