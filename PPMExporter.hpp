#pragma once
#include <string>
#include <vector>

class PPMExporter
{
public:
    static void ExportP3(const std::string& outFileLoc, size_t width, size_t height, const std::vector<float>& pixelData);
};

