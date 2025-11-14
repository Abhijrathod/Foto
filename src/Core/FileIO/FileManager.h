#pragma once
#include "../Memory/BufferManager.h"
#include <string>

class FileManager {
public:
    static bool LoadImage(const std::string& filepath, BufferManager::Buffer& outBuffer);
    static bool SaveImage(const std::string& filepath, const BufferManager::Buffer& buffer);
    
    static bool LoadPSD(const std::string& filepath, class ImageEngine* engine);
    static bool SavePSD(const std::string& filepath, class ImageEngine* engine);
};

