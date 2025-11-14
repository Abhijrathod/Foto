#pragma once
#include "LayerManager.h"
#include "HistoryManager.h"
#include "../Memory/BufferManager.h"
#include <cstdint>
#include <string>
#include <memory>

// Main image processing engine
class ImageEngine {
public:
    ImageEngine();
    ~ImageEngine();

    // Document management
    bool CreateNew(uint32_t width, uint32_t height, const std::string& name = "Untitled");
    bool LoadFromFile(const std::string& filepath);
    bool SaveToFile(const std::string& filepath);
    
    uint32_t GetWidth() const { return width_; }
    uint32_t GetHeight() const { return height_; }
    const std::string& GetName() const { return name_; }
    void SetName(const std::string& name) { name_ = name; }

    // Layer management
    LayerManager& GetLayerManager() { return layerManager_; }
    const LayerManager& GetLayerManager() const { return layerManager_; }

    // History management
    HistoryManager& GetHistoryManager() { return historyManager_; }
    const HistoryManager& GetHistoryManager() const { return historyManager_; }

    // Composite all layers into final image
    BufferManager::Buffer GetCompositeImage() const;

    // Apply filter to active layer
    void ApplyFilterToActiveLayer(class FilterBase* filter);

private:
    uint32_t width_ = 0;
    uint32_t height_ = 0;
    std::string name_;
    LayerManager layerManager_;
    HistoryManager historyManager_;
};

