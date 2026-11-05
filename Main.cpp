#include "SyntheticSensors.h"
#include "SensorCsvWriter.h"
#include <iostream>
#include <exception>

/**
 * Run in powershell:
 * "g++ -std=c++17 -O2 main.cpp SyntheticSensors.cpp SensorCsvWriter.cpp -o sensor_gen.exe"
 *
 * ".\sensor_gen.exe"
 *
 * use
 * double v = sensor.next();
 *
 * for (int...) {
 *      auto s = stream.next();
 * }
 *
 */

int main()
{
    try
    {
        const double fs = 100.0;

        RainConfig rain;
        rain.intensity = 0.25;

        SensorStream stream(fs, rain);

        // ---- 1) prove the generator works, even before we touch files ----
        std::cout << "First 5 generated samples:\n";
        for (int i = 0; i < 5; ++i)
        {
            SensorSample s = stream.next();
            std::cout << "  t=" << s.t
                      << "  piezo=" << s.piezo
                      << "  T=" << s.temperature
                      << "  H=" << s.humidity
                      << "  L=" << s.light << "\n";
        }

        // ---- 2) now write a file ----------------------------------------
        SensorCsvWriter writer("sensor_data.csv");

        const std::size_t blockSize = 1000;
        const int numBlocks = 6;

        for (int b = 0; b < numBlocks; ++b)
        {
            writeNextBlock(stream, writer, blockSize);
            if (b == 2)
            {
                stream.piezo().setIntensity(0.9);
                std::cout << "[sim] rain intensity -> 0.9\n";
            }
            std::cout << "Wrote block " << (b + 1) << "/" << numBlocks << "\n";
        }

        writer.flush();
        std::cout << "Done -> sensor_data.csv\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
}