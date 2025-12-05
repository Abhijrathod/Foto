#define NOMINMAX
#include "FileManager.h"
#include "ImageCodecs.h"
#include "PSDFormat.h"
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
    if (!buffer.data || buffer.width == 0 || buffer.height == 0) {
        return false;
    }

    // Convert to wide string for WIC
    std::wstring wpath(filepath.begin(), filepath.end());

    ImageData img;
    img.width = buffer.width;
    img.height = buffer.height;
    img.pixels.assign(buffer.data, buffer.data + buffer.size);
    img.valid = true;

    return ImageCodecs::SaveImage(wpath.c_str(), img);
}

bool FileManager::LoadPSD(const std::string& filepath, ImageEngine* engine) {
    if (!engine) {
        return false;
    }
    return PSDFormat::Load(filepath, engine);
}

bool FileManager::SavePSD(const std::string& filepath, ImageEngine* engine) {
    if (!engine) {
        return false;
    }
    return PSDFormat::Save(filepath, engine);
}

