#include "GPURenderer.h"
#include <d3dcompiler.h>
#include <cstring>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using Microsoft::WRL::ComPtr;

// Simple shader for rendering a textured quad
static const char* kSimpleShader = R"(
struct VSOut {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

VSOut VS(uint id : SV_VertexID) {
    float2 verts[3] = {
        float2(-1.0f,-1.0f),
        float2(-1.0f, 3.0f),
        float2( 3.0f,-1.0f)
    };
    VSOut o;
    o.pos = float4(verts[id], 0, 1);
    o.uv = (verts[id] + 1.0f) * 0.5f;
    o.uv.y = 1.0f - o.uv.y; // Flip Y
    return o;
}

Texture2D tex0 : register(t0);
SamplerState samp : register(s0);

float4 PS(VSOut inp) : SV_TARGET {
    return tex0.Sample(samp, inp.uv);
}
)";

GPURenderer::GPURenderer(HWND hwnd) : hwnd_(hwnd) {
}

GPURenderer::~GPURenderer() {
    Shutdown();
}

bool GPURenderer::Initialize() {
    RECT rc;
    GetClientRect(hwnd_, &rc);
    width_ = rc.right - rc.left;
    height_ = rc.bottom - rc.top;

    if (width_ == 0) width_ = 1;
    if (height_ == 0) height_ = 1;

    // Create D3D11 device
    UINT flags = 0;
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL level;
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &device_,
        &level,
        &context_);

    if (FAILED(hr)) return false;

    // Get DXGI factory
    ComPtr<IDXGIDevice> dxgiDevice;
    device_.As(&dxgiDevice);

    ComPtr<IDXGIAdapter> adapter;
    dxgiDevice->GetAdapter(&adapter);

    ComPtr<IDXGIFactory2> factory;
    adapter->GetParent(IID_PPV_ARGS(&factory));

    // Create swap chain
    DXGI_SWAP_CHAIN_DESC1 sd = {};
    sd.Width = width_;
    sd.Height = height_;
    sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 2;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    hr = factory->CreateSwapChainForHwnd(
        device_.Get(),
        hwnd_,
        &sd,
        nullptr,
        nullptr,
        &swapChain_);

    if (FAILED(hr)) return false;

    // Create render target view
    if (!CreateRenderTarget()) return false;

    // Create shaders
    if (!CreateShaders()) return false;

    // Create sampler state
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    hr = device_->CreateSamplerState(&sampDesc, &samplerState_);

    if (FAILED(hr)) return false;

    return true;
}

void GPURenderer::Shutdown() {
    if (context_) {
        context_->ClearState();
        context_.Reset();
    }

    samplerState_.Reset();
    srv_.Reset();
    texture_.Reset();
    ps_.Reset();
    vs_.Reset();
    rtv_.Reset();
    swapChain_.Reset();
    device_.Reset();
}

void GPURenderer::Render(const BufferManager::Buffer& buffer) {
    if (!device_ || !context_ || !swapChain_) return;

    // Update texture with buffer data
    UpdateTexture(buffer);

    // Set render target
    context_->OMSetRenderTargets(1, rtv_.GetAddressOf(), nullptr);

    // Clear background
    float clearColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    context_->ClearRenderTargetView(rtv_.Get(), clearColor);

    // Set viewport
    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<float>(width_);
    vp.Height = static_cast<float>(height_);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    context_->RSSetViewports(1, &vp);

    // Set shaders and resources
    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context_->VSSetShader(vs_.Get(), nullptr, 0);
    context_->PSSetShader(ps_.Get(), nullptr, 0);

    if (srv_) {
        ID3D11ShaderResourceView* srvs[] = { srv_.Get() };
        context_->PSSetShaderResources(0, 1, srvs);
    }

    ID3D11SamplerState* samplers[] = { samplerState_.Get() };
    context_->PSSetSamplers(0, 1, samplers);

    // Draw fullscreen triangle
    context_->Draw(3, 0);

    // Present
    swapChain_->Present(1, 0);

    // Unbind SRV
    ID3D11ShaderResourceView* nullSRV[] = { nullptr };
    context_->PSSetShaderResources(0, 1, nullSRV);
}

void GPURenderer::Resize(uint32_t width, uint32_t height) {
    if (!swapChain_) return;

    width_ = width > 0 ? width : 1;
    height_ = height > 0 ? height : 1;

    // Release old render target
    rtv_.Reset();
    context_->ClearState();

    // Resize swap chain
    HRESULT hr = swapChain_->ResizeBuffers(
        0,
        width_,
        height_,
        DXGI_FORMAT_UNKNOWN,
        0);

    if (FAILED(hr)) return;

    // Recreate render target
    CreateRenderTarget();
}

bool GPURenderer::CreateRenderTarget() {
    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr)) return false;

    hr = device_->CreateRenderTargetView(backBuffer.Get(), nullptr, &rtv_);
    return SUCCEEDED(hr);
}

bool GPURenderer::CreateShaders() {
    ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;

    // Compile vertex shader
    HRESULT hr = D3DCompile(
        kSimpleShader,
        strlen(kSimpleShader),
        nullptr,
        nullptr,
        nullptr,
        "VS",
        "vs_5_0",
        0,
        0,
        &vsBlob,
        &errorBlob);

    if (FAILED(hr)) return false;

    hr = device_->CreateVertexShader(
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        nullptr,
        &vs_);

    if (FAILED(hr)) return false;

    // Compile pixel shader
    hr = D3DCompile(
        kSimpleShader,
        strlen(kSimpleShader),
        nullptr,
        nullptr,
        nullptr,
        "PS",
        "ps_5_0",
        0,
        0,
        &psBlob,
        &errorBlob);

    if (FAILED(hr)) return false;

    hr = device_->CreatePixelShader(
        psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        nullptr,
        &ps_);

    return SUCCEEDED(hr);
}

void GPURenderer::UpdateTexture(const BufferManager::Buffer& buffer) {
    if (!buffer.data || buffer.width == 0 || buffer.height == 0) return;

    // Recreate texture if dimensions changed
    if (!texture_ || buffer.width != width_ || buffer.height != height_) {
        texture_.Reset();
        srv_.Reset();

        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = buffer.width;
        texDesc.Height = buffer.height;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        HRESULT hr = device_->CreateTexture2D(&texDesc, nullptr, &texture_);
        if (FAILED(hr)) return;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = texDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        hr = device_->CreateShaderResourceView(texture_.Get(), &srvDesc, &srv_);
        if (FAILED(hr)) return;
    }

    // Update texture data
    context_->UpdateSubresource(
        texture_.Get(),
        0,
        nullptr,
        buffer.data,
        buffer.width * 4,
        0);
}

