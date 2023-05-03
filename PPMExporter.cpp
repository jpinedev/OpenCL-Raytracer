#include "PPMExporter.hpp"

#include <chrono>
#include <fstream>
#include <iostream>

void PPMExporter::ExportP3(const std::string& outFileLoc, size_t width, size_t height, const std::vector<float>& pixelData)
{
    std::cout << "Exporting to file '" << outFileLoc << "'...\n";

    auto startTime = std::chrono::high_resolution_clock::now();

    std::ofstream op(outFileLoc);

    op << "P3" << "\n";
    op << width << " " << height << "\n";
    op << "255\n";
    for (size_t ii = 0; ii < height * width; ++ii) {
        op << std::min(255, (int)floorf(pixelData[ii * 3] * 255.f)) << " ";
        op << std::min(255, (int)floorf(pixelData[ii * 3 + 1] * 255.f)) << " ";
        op << std::min(255, (int)floorf(pixelData[ii * 3 + 2] * 255.f)) << std::endl;
    }
    op.close();

    auto endTime = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    std::cout << "Export finished in " << duration.count() << "ms.\n";
}
