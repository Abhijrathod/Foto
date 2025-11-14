#define NOMINMAX
#include "FileManager.h"
#include "ImageCodecs.h"
#include "../Engine/ImageEngine.h"
#include <cstring>
#include <algorithm>

bool FileManager::LoadImage(const std::string& filepath, BufferManager::Buffer& outBuffer) {
    // Convert to wide string for WIC
    std::wstring wpath(filepath.begin(), filepath.end());
    
    ImageData img;
    bool result = ImageCodecs::LoadImage(wpath.c_str(), img);
    if (result && img.valid) {
        outBuffer = BufferManager::Create(img.width, img.height);
        std::memcpy(outBuffer.data, img.pixels.data(), outBuffer.size);
        return true;
    }
    return false;
}

bool FileManager::SaveImage(const std::string& filepath, const BufferManager::Buffer& buffer) {
    // TODO: Implement image saving
    return false;
}

bool FileManager::LoadPSD(const std::string& filepath, ImageEngine* engine) {
    // TODO: Implement PSD loading
    return false;
}

bool FileManager::SavePSD(const std::string& filepath, ImageEngine* engine) {
    // TODO: Implement PSD saving
    return false;
}

