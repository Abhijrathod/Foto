#pragma once
#include <windows.h>

// MUST BE FIRST for DXGI types like IDXGISwapChain1
#include <dxgi.h>
#include <dxgi1_2.h>

#include <d3d11.h>
#include <wrl.h>

class DXCanvas {
public:
    DXCanvas(HWND hostWnd);
    ~DXCanvas();

    bool Initialize();
    void Resize(UINT width, UINT height);
    void Render();

private:
    HWND hostWnd_;

    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv_;
};
