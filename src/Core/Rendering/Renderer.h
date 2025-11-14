#pragma once
#include "../Memory/BufferManager.h"

// Base renderer interface
class Renderer {
public:
    virtual ~Renderer() = default;
    
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    
    virtual void Render(const BufferManager::Buffer& buffer) = 0;
    virtual void Resize(uint32_t width, uint32_t height) = 0;
    
    virtual bool IsGPUAccelerated() const { return false; }
};

