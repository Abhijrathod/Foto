#pragma once
#include "Renderer.h"

class CPURenderer : public Renderer {
public:
    CPURenderer();
    ~CPURenderer();

    bool Initialize() override;
    void Shutdown() override;
    
    void Render(const BufferManager::Buffer& buffer) override;
    void Resize(uint32_t width, uint32_t height) override;
    
    bool IsGPUAccelerated() const override { return false; }
};

