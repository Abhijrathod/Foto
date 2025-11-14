#pragma once
#include "../Memory/BufferManager.h"
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <mutex>

enum class BlendMode {
    Normal,
    Multiply,
    Screen,
    Overlay,
    SoftLight,
    HardLight,
    ColorDodge,
    ColorBurn,
    Darken,
    Lighten,
    Difference,
    Exclusion
};

class Layer {
public:
    Layer(uint32_t width, uint32_t height, const std::string& name = "Layer");
    ~Layer();

    uint32_t GetWidth() const { return width_; }
    uint32_t GetHeight() const { return height_; }
    const std::string& GetName() const { return name_; }
    void SetName(const std::string& name) { name_ = name; }

    bool IsVisible() const { return visible_; }
    void SetVisible(bool visible) { visible_ = visible; }

    float GetOpacity() const { return opacity_; }
    void SetOpacity(float opacity) { 
        opacity_ = (opacity < 0.0f) ? 0.0f : (opacity > 1.0f) ? 1.0f : opacity;
    }

    BlendMode GetBlendMode() const { return blendMode_; }
    void SetBlendMode(BlendMode mode) { blendMode_ = mode; }

    BufferManager::Buffer& GetBuffer() { return buffer_; }
    const BufferManager::Buffer& GetBuffer() const { return buffer_; }

    bool IsLocked() const { return locked_; }
    void SetLocked(bool locked) { locked_ = locked; }

private:
    BufferManager::Buffer buffer_;
    uint32_t width_ = 0;
    uint32_t height_ = 0;
    std::string name_;
    bool visible_ = true;
    float opacity_ = 1.0f;
    BlendMode blendMode_ = BlendMode::Normal;
    bool locked_ = false;
};

class LayerManager {
public:
    LayerManager();
    ~LayerManager();

    // Layer management
    Layer* CreateLayer(uint32_t width, uint32_t height, const std::string& name = "Layer");
    void DeleteLayer(size_t index);
    void DeleteLayer(Layer* layer);
    
    Layer* GetLayer(size_t index);
    const Layer* GetLayer(size_t index) const;
    Layer* GetActiveLayer() { return activeLayer_; }
    void SetActiveLayer(size_t index);
    void SetActiveLayer(Layer* layer);

    size_t GetLayerCount() const { return layers_.size(); }
    void MoveLayer(size_t from, size_t to);
    void DuplicateLayer(size_t index);

    // Composite all visible layers into a single buffer
    BufferManager::Buffer CompositeLayers(uint32_t width, uint32_t height) const;

private:
    std::vector<std::unique_ptr<Layer>> layers_;
    Layer* activeLayer_ = nullptr;
    mutable std::mutex mutex_;
};

