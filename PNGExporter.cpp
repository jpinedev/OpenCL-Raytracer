#include "PNGExporter.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>

void PNGExporter::Export(const std::string& outFileLoc, size_t width, size_t height, const std::vector<float>& pixelData)
{
    std::vector<unsigned char> pixelsUnsigned(pixelData.size());
    for (size_t i = 0; i < pixelData.size(); ++i) {
        pixelsUnsigned[i] = static_cast<unsigned char>(pixelData[i] * 255.0f);
    }

    cv::Mat image(height, width, CV_8UC3);
    std::memcpy(image.data, pixelsUnsigned.data(), pixelsUnsigned.size() * sizeof(unsigned char));

    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_ERROR); // ignore info logs about parallelism
    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
    cv::flip(image, image, 0);

    if (!cv::imwrite(outFileLoc, image)) {
        std::cerr << "Error writing file: " << outFileLoc << std::endl;
        return;
    }

    std::cout << "Exported to '" << outFileLoc << "'" << std::endl;
}
