#pragma once
#include "Renderer.h"
#include <windows.h>

class GPURenderer : public Renderer {
public:
    GPURenderer(HWND hwnd);
    ~GPURenderer();

    bool Initialize() override;
    void Shutdown() override;
    
    void Render(const BufferManager::Buffer& buffer) override;
    void Resize(uint32_t width, uint32_t height) override;
    
    bool IsGPUAccelerated() const override { return true; }

private:
    HWND hwnd_;
    // DirectX resources would go here
};

