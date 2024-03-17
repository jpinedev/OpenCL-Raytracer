#pragma once
#include <string>
#include <vector>
class PNGExporter
{
public:
    static void Export(const std::string& outFileLoc, size_t width, size_t height, const std::vector<float>& pixelData);
};

