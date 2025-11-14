#include "LayerManager.h"
#include "../Math/ColorSpace.h"
#include <algorithm>

Layer::Layer(uint32_t width, uint32_t height, const std::string& name)
    : width_(width), height_(height), name_(name) {
    buffer_ = BufferManager::Create(width, height);
    BufferManager::Clear(buffer_, 0, 0, 0, 0); // Transparent
}

Layer::~Layer() {
    BufferManager::Destroy(buffer_);
}

LayerManager::LayerManager() {
}

LayerManager::~LayerManager() {
}

Layer* LayerManager::CreateLayer(uint32_t width, uint32_t height, const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto layer = std::make_unique<Layer>(width, height, name);
    Layer* ptr = layer.get();
    layers_.push_back(std::move(layer));
    
    if (!activeLayer_) {
        activeLayer_ = ptr;
    }
    
    return ptr;
}

void LayerManager::DeleteLayer(size_t index) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (index >= layers_.size()) return;
    
    Layer* toDelete = layers_[index].get();
    if (activeLayer_ == toDelete) {
        activeLayer_ = nullptr;
        if (index > 0 && layers_.size() > 1) {
            activeLayer_ = layers_[index - 1].get();
        } else if (layers_.size() > 1) {
            activeLayer_ = layers_[1].get();
        }
    }
    
    layers_.erase(layers_.begin() + index);
    
    if (!activeLayer_ && !layers_.empty()) {
        activeLayer_ = layers_[0].get();
    }
}

void LayerManager::DeleteLayer(Layer* layer) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(layers_.begin(), layers_.end(),
        [layer](const std::unique_ptr<Layer>& l) { return l.get() == layer; });
    
    if (it != layers_.end()) {
        size_t index = it - layers_.begin();
        DeleteLayer(index);
    }
}

Layer* LayerManager::GetLayer(size_t index) {
    std::lock_guard<std::mutex> lock(mutex_);
    return (index < layers_.size()) ? layers_[index].get() : nullptr;
}

const Layer* LayerManager::GetLayer(size_t index) const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex_));
    return (index < layers_.size()) ? layers_[index].get() : nullptr;
}

void LayerManager::SetActiveLayer(size_t index) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (index < layers_.size()) {
        activeLayer_ = layers_[index].get();
    }
}

void LayerManager::SetActiveLayer(Layer* layer) {
    std::lock_guard<std::mutex> lock(mutex_);
    activeLayer_ = layer;
}

void LayerManager::MoveLayer(size_t from, size_t to) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (from >= layers_.size() || to >= layers_.size()) return;
    if (from == to) return;
    
    auto layer = std::move(layers_[from]);
    layers_.erase(layers_.begin() + from);
    
    if (to > from) to--;
    layers_.insert(layers_.begin() + to, std::move(layer));
}

void LayerManager::DuplicateLayer(size_t index) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (index >= layers_.size()) return;
    
    Layer* src = layers_[index].get();
    auto dup = std::make_unique<Layer>(src->GetWidth(), src->GetHeight(), src->GetName() + " Copy");
    BufferManager::Copy(src->GetBuffer(), dup->GetBuffer());
    dup->SetOpacity(src->GetOpacity());
    dup->SetBlendMode(src->GetBlendMode());
    
    layers_.insert(layers_.begin() + index + 1, std::move(dup));
}

BufferManager::Buffer LayerManager::CompositeLayers(uint32_t width, uint32_t height) const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex_));
    
    BufferManager::Buffer result = BufferManager::Create(width, height);
    BufferManager::Clear(result, 0, 0, 0, 0);
    
    for (const auto& layer : layers_) {
        if (!layer->IsVisible()) continue;
        
        // Simple alpha compositing (can be extended for blend modes)
        float opacity = layer->GetOpacity();
        const auto& src = layer->GetBuffer();
        
        for (uint32_t y = 0; y < std::min(height, layer->GetHeight()); y++) {
            for (uint32_t x = 0; x < std::min(width, layer->GetWidth()); x++) {
                const uint8_t* srcPixel = BufferManager::GetPixel(src, x, y);
                uint8_t* dstPixel = BufferManager::GetPixel(result, x, y);
                
                if (srcPixel && dstPixel) {
                    float srcA = srcPixel[3] / 255.0f * opacity;
                    float dstA = dstPixel[3] / 255.0f;
                    float outA = srcA + dstA * (1.0f - srcA);
                    
                    if (outA > 0.0f) {
                        for (int i = 0; i < 3; i++) {
                            float srcC = srcPixel[i] / 255.0f;
                            float dstC = dstPixel[i] / 255.0f;
                            dstPixel[i] = static_cast<uint8_t>((srcC * srcA + dstC * dstA * (1.0f - srcA)) / outA * 255.0f);
                        }
                        dstPixel[3] = static_cast<uint8_t>(outA * 255.0f);
                    }
                }
            }
        }
    }
    
    return result;
}

