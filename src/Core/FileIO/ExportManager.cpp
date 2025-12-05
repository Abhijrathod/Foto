#include "ExportManager.h"
#include "ImageCodecs.h"
#include "FileManager.h"
#include <algorithm>

bool ExportManager::Export(const std::string& filepath, const BufferManager::Buffer& buffer, Format format) {
    return ExportWithOptions(filepath, buffer, format, 90);
}

bool ExportManager::ExportWithOptions(const std::string& filepath, const BufferManager::Buffer& buffer,
                                      Format format, int quality) {
    if (!buffer.data || buffer.width == 0 || buffer.height == 0) {
        return false;
    }

    // Ensure quality is in valid range [0, 100]
    quality = std::max(0, std::min(100, quality));

    // Build filepath with correct extension
    std::string outputPath = filepath;

    // Remove existing extension if present
    size_t dotPos = outputPath.find_last_of('.');
    if (dotPos != std::string::npos) {
        outputPath = outputPath.substr(0, dotPos);
    }

    // Add appropriate extension based on format
    switch (format) {
        case Format::PNG:
            outputPath += ".png";
            break;
        case Format::JPEG:
            outputPath += ".jpg";
            break;
        case Format::BMP:
            outputPath += ".bmp";
            break;
        case Format::TIFF:
            outputPath += ".tiff";
            break;
        default:
            outputPath += ".png";
            break;
    }

    // For now, use FileManager::SaveImage which handles format detection by extension
    // Note: Quality parameter is currently not customizable per export,
    // but the infrastructure is in place for future enhancement
    return FileManager::SaveImage(outputPath, buffer);
}

