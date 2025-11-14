#pragma once
#include "../Memory/BufferManager.h"
#include <string>

class ExportManager {
public:
    enum class Format {
        PNG,
        JPEG,
        BMP,
        TIFF
    };
    
    static bool Export(const std::string& filepath, const BufferManager::Buffer& buffer, Format format);
    static bool ExportWithOptions(const std::string& filepath, const BufferManager::Buffer& buffer, 
                                 Format format, int quality = 90);
};

