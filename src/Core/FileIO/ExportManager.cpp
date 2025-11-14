#include "ExportManager.h"
#include "ImageCodecs.h"

bool ExportManager::Export(const std::string& filepath, const BufferManager::Buffer& buffer, Format format) {
    return ExportWithOptions(filepath, buffer, format, 90);
}

bool ExportManager::ExportWithOptions(const std::string& filepath, const BufferManager::Buffer& buffer,
                                      Format format, int quality) {
    // TODO: Implement export with format-specific options
    return false;
}

