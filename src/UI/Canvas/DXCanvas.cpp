#include "DXCanvas.h"

#include <dxgi1_2.h>
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;

DXCanvas::DXCanvas(HWND h) : hostWnd_(h) {}

DXCanvas::~DXCanvas() {
    if (context_) context_->ClearState();
}

bool DXCanvas::Initialize() {
    RECT rc;
    GetClientRect(hostWnd_, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT flags = 0;
    D3D_FEATURE_LEVEL fl;
    D3D_FEATURE_LEVEL fls[] = { D3D_FEATURE_LEVEL_11_0 };

    HRESULT hr = D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE,
        nullptr, flags,
        fls, ARRAYSIZE(fls),
        D3D11_SDK_VERSION,
        &device_, &fl, &context_);

    if (FAILED(hr)) return false;

    ComPtr<IDXGIDevice> dxgiDevice;
    device_.As(&dxgiDevice);

    ComPtr<IDXGIAdapter> adapter;
    dxgiDevice->GetAdapter(&adapter);

    ComPtr<IDXGIFactory2> factory;
    adapter->GetParent(__uuidof(IDXGIFactory2), &factory);

    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    hr = factory->CreateSwapChainForHwnd(
        device_.Get(),
        hostWnd_,
        &desc,
        nullptr,
        nullptr,
        &swapChain_
    );

    if (FAILED(hr)) return false;

    ComPtr<ID3D11Texture2D> backBuffer;
    swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer));

    device_->CreateRenderTargetView(backBuffer.Get(), nullptr, &rtv_);

    return true;
}

void DXCanvas::Resize(UINT w, UINT h) {
    if (!swapChain_) return;

    context_->OMSetRenderTargets(0, nullptr, nullptr);
    rtv_.Reset();

    swapChain_->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);

    ComPtr<ID3D11Texture2D> backBuffer;
    swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    device_->CreateRenderTargetView(backBuffer.Get(), nullptr, &rtv_);
}

void DXCanvas::Render() {
    if (!context_ || !rtv_) return;

    float c[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    context_->OMSetRenderTargets(1, rtv_.GetAddressOf(), nullptr);
    context_->ClearRenderTargetView(rtv_.Get(), c);

    swapChain_->Present(1, 0);
}
