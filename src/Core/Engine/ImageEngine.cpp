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
    // TODO: Implement file loading via FileManager
    return false;
}

bool ImageEngine::SaveToFile(const std::string& filepath) {
    // TODO: Implement file saving via FileManager
    return false;
}

BufferManager::Buffer ImageEngine::GetCompositeImage() const {
    return layerManager_.CompositeLayers(width_, height_);
}

void ImageEngine::ApplyFilterToActiveLayer(FilterBase* filter) {
    Layer* layer = layerManager_.GetActiveLayer();
    if (!layer || !filter) return;
    
    // TODO: Apply filter to layer buffer
}

