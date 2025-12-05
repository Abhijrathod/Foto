#include "ImageEngine.h"
#include "../FileIO/FileManager.h"
#include "../Filters/FilterBase.h"

ImageEngine::ImageEngine() {
}

ImageEngine::~ImageEngine() {
}

bool ImageEngine::CreateNew(uint32_t width, uint32_t height, const std::string& name) {
    width_ = width;
    height_ = height;
    name_ = name;
    
    layerManager_.CreateLayer(width, height, "Background");
    historyManager_.Clear();
    
    return true;
}

bool ImageEngine::LoadFromFile(const std::string& filepath) {
    // Check file extension to determine format
    size_t dotPos = filepath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return false; // No extension
    }

    std::string extension = filepath.substr(dotPos);

    // Convert to lowercase for comparison
    for (char& c : extension) {
        c = tolower(c);
    }

    // Load PSD files using PSDFormat
    if (extension == ".psd") {
        return FileManager::LoadPSD(filepath, this);
    }

    // Load regular image files (PNG, JPG, BMP, etc.)
    BufferManager::Buffer buffer;
    if (!FileManager::LoadImage(filepath, buffer)) {
        return false;
    }

    // Create new document with loaded image dimensions
    if (!CreateNew(buffer.width, buffer.height, filepath)) {
        BufferManager::Destroy(buffer);
        return false;
    }

    // Copy loaded data to the background layer
    Layer* bgLayer = layerManager_.GetLayer(0);
    if (bgLayer) {
        BufferManager::Copy(buffer, bgLayer->GetBuffer());
    }

    BufferManager::Destroy(buffer);
    return true;
}

bool ImageEngine::SaveToFile(const std::string& filepath) {
    // Check file extension to determine format
    size_t dotPos = filepath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return false; // No extension
    }

    std::string extension = filepath.substr(dotPos);

    // Convert to lowercase for comparison
    for (char& c : extension) {
        c = tolower(c);
    }

    // Save as PSD if extension is .psd
    if (extension == ".psd") {
        return FileManager::SavePSD(filepath, this);
    }

    // For other formats, save the composite (flattened) image
    BufferManager::Buffer composite = GetCompositeImage();
    if (!composite.data) {
        return false;
    }

    bool result = FileManager::SaveImage(filepath, composite);
    BufferManager::Destroy(composite);

    return result;
}

BufferManager::Buffer ImageEngine::GetCompositeImage() const {
    return layerManager_.CompositeLayers(width_, height_);
}

void ImageEngine::ApplyFilterToActiveLayer(FilterBase* filter) {
    Layer* layer = layerManager_.GetActiveLayer();
    if (!layer || !filter) return;

    BufferManager::Buffer& layerBuffer = layer->GetBuffer();

    // Check if filter can be applied
    if (!filter->CanApply(layerBuffer)) {
        return;
    }

    // Apply the filter to the layer buffer
    filter->Apply(layerBuffer);

    // TODO: Add to history for undo/redo support
    // historyManager_.AddAction(...);
}

