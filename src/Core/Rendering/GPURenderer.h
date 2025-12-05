#pragma once
#include "Renderer.h"
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

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
    bool CreateRenderTarget();
    bool CreateShaders();
    void UpdateTexture(const BufferManager::Buffer& buffer);

    HWND hwnd_;
    uint32_t width_ = 0;
    uint32_t height_ = 0;

    // DirectX resources
    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv_;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vs_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> ps_;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState_;
};

