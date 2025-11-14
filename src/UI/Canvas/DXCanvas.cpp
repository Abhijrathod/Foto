#define NOMINMAX
#include "DXCanvas.h"
#include <d3dcompiler.h>
#include <algorithm>
#include <cstring>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using Microsoft::WRL::ComPtr;

static const char* kShader = R"(
cbuffer TransformCB : register(b0) {
    float panX;
    float panY;
    float zoom;
    float padding1;
    float canvasWidth;
    float canvasHeight;
    float2 padding2;
};

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
    
    // Apply zoom and pan transformation
    float2 uv = (verts[id] + 1.0f) * 0.5f;
    uv = (uv - 0.5f) * zoom + 0.5f;
    uv = uv - float2(panX, panY) / float2(canvasWidth, canvasHeight);
    
    o.uv = uv;
    return o;
}

Texture2D tex0 : register(t0);
SamplerState samp : register(s0);

float4 PS(VSOut inp) : SV_TARGET {
    // Clamp UV to avoid sampling outside texture
    if (inp.uv.x < 0.0 || inp.uv.x > 1.0 || inp.uv.y < 0.0 || inp.uv.y > 1.0) {
        return float4(0.2, 0.2, 0.2, 1.0); // Dark gray background
    }
    return tex0.Sample(samp, inp.uv);
}
)";

DXCanvas::DXCanvas(HWND hostWnd) : hostWnd_(hostWnd) {}

DXCanvas::~DXCanvas() {
    if (context_) context_->ClearState();
}

bool DXCanvas::Initialize() {
    RECT rc; 
    GetClientRect(hostWnd_, &rc);
LONG w = rc.right - rc.left;
LONG h = rc.bottom - rc.top;

width_  = (UINT)(w > 0 ? w : 1);
height_ = (UINT)(h > 0 ? h : 1);


    UINT flags = 0;

    D3D_FEATURE_LEVEL lvl;
    HRESULT hr = D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE,
        nullptr, flags,
        nullptr, 0,
        D3D11_SDK_VERSION,
        &device_, &lvl, &context_);

    if (FAILED(hr)) return false;

    ComPtr<IDXGIDevice> dxgi;
    device_.As(&dxgi);

    ComPtr<IDXGIAdapter> ad;
    dxgi->GetAdapter(&ad);

    ComPtr<IDXGIFactory2> factory;
    ad->GetParent(IID_PPV_ARGS(&factory));

    DXGI_SWAP_CHAIN_DESC1 sd = {};
    sd.Width = width_;
    sd.Height = height_;
    sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 2;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    factory->CreateSwapChainForHwnd(
        device_.Get(), hostWnd_, &sd, nullptr, nullptr, &swapChain_);

    ComPtr<ID3D11Texture2D> back;
    swapChain_->GetBuffer(0, IID_PPV_ARGS(&back));
    device_->CreateRenderTargetView(back.Get(), nullptr, &rtv_);

    // Shaders
    ComPtr<ID3DBlob> vs, ps, err;
    D3DCompile(kShader, strlen(kShader), nullptr, nullptr, nullptr,
        "VS", "vs_5_0", 0, 0, &vs, &err);

    D3DCompile(kShader, strlen(kShader), nullptr, nullptr, nullptr,
        "PS", "ps_5_0", 0, 0, &ps, &err);

    device_->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, &vs_);
    device_->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr, &ps_);

    // Sampler
    D3D11_SAMPLER_DESC s = {};
    s.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    s.AddressU = s.AddressV = s.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    device_->CreateSamplerState(&s, &samplerState_);

    // Constant buffer for zoom/pan (16-byte aligned)
    struct TransformCB {
        float panX;
        float panY;
        float zoom;
        float padding1;
        float canvasWidth;
        float canvasHeight;
        float padding2[2];
    };
    static_assert(sizeof(TransformCB) % 16 == 0, "TransformCB must be 16-byte aligned");
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.ByteWidth = sizeof(TransformCB);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = device_->CreateBuffer(&cbDesc, nullptr, &transformCB_);
    if (FAILED(hr)) return false;

    InitCanvasBuffer(width_, height_);
    UploadCanvasToGPU();

    return true;
}

bool DXCanvas::InitCanvasBuffer(UINT w, UINT h) {
    width_ = w; height_ = h;
    cpuBuffer_.assign(w * h * 4, 0);

    for (UINT i = 0; i < w * h; i++) {
        cpuBuffer_[4 * i + 0] = 40;
        cpuBuffer_[4 * i + 1] = 40;
        cpuBuffer_[4 * i + 2] = 40;
        cpuBuffer_[4 * i + 3] = 255;
    }

    D3D11_TEXTURE2D_DESC td = {};
    td.Width = w;
    td.Height = h;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    device_->CreateTexture2D(&td, nullptr, &canvasTexture_);

    D3D11_SHADER_RESOURCE_VIEW_DESC sd = {};
    sd.Format = td.Format;
    sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    sd.Texture2D.MipLevels = 1;
    device_->CreateShaderResourceView(canvasTexture_.Get(), &sd, &canvasSRV_);

    return true;
}

void DXCanvas::UploadCanvasToGPU() {
    if (!canvasTexture_) return;

    context_->UpdateSubresource(
        canvasTexture_.Get(),
        0,
        nullptr,
        cpuBuffer_.data(),
        width_ * 4,
        0
    );
}

void DXCanvas::UploadCanvasRegion(UINT x, UINT y, UINT w, UINT h) {
    if (!canvasTexture_) return;

    D3D11_BOX box = {};
    box.left = x;
    box.top = y;
    box.front = 0;
    box.right = x + w;
    box.bottom = y + h;
    box.back = 1;

    const uint8_t* ptr =
        cpuBuffer_.data() + (y * width_ + x) * 4;

    context_->UpdateSubresource(
        canvasTexture_.Get(),
        0,
        &box,
        ptr,
        width_ * 4,
        0
    );
}

void DXCanvas::DrawCircleToCPU(int cx, int cy, int radius,
    uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    int r2 = radius * radius;

    int x0 = std::max(0, cx - radius);
    int x1 = std::min<int>(width_ - 1, cx + radius);
    int y0 = std::max(0, cy - radius);
    int y1 = std::min<int>(height_ - 1, cy + radius);

    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++)
        {
            int dx = x - cx;
            int dy = y - cy;
            if (dx * dx + dy * dy > r2) continue;

            size_t idx = (y * width_ + x) * 4;

            cpuBuffer_[idx + 2] = r;
            cpuBuffer_[idx + 1] = g;
            cpuBuffer_[idx + 0] = b;
            cpuBuffer_[idx + 3] = a;
        }

    UploadCanvasRegion(x0, y0, x1 - x0 + 1, y1 - y0 + 1);
}

void DXCanvas::Resize(UINT w, UINT h) {
    swapChain_->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);

    ComPtr<ID3D11Texture2D> back;
    swapChain_->GetBuffer(0, IID_PPV_ARGS(&back));
    device_->CreateRenderTargetView(back.Get(), nullptr, &rtv_);

    InitCanvasBuffer(w, h);
    UploadCanvasToGPU();
}

bool DXCanvas::LoadImageFromBuffer(const uint8_t* pixels, UINT width, UINT height) {
    if (!pixels || width == 0 || height == 0) return false;

    width_ = width;
    height_ = height;
    cpuBuffer_.assign(pixels, pixels + (width * height * 4));

    // Recreate texture with new dimensions
    canvasTexture_.Reset();
    canvasSRV_.Reset();

    D3D11_TEXTURE2D_DESC td = {};
    td.Width = width;
    td.Height = height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = device_->CreateTexture2D(&td, nullptr, &canvasTexture_);
    if (FAILED(hr)) return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC sd = {};
    sd.Format = td.Format;
    sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    sd.Texture2D.MipLevels = 1;
    hr = device_->CreateShaderResourceView(canvasTexture_.Get(), &sd, &canvasSRV_);
    if (FAILED(hr)) return false;

    UploadCanvasToGPU();
    return true;
}

void DXCanvas::ZoomAtPoint(float zoom, float x, float y) {
    float oldZoom = zoom_;
    zoom_ = std::max(0.1f, std::min(10.0f, zoom));
    
    // Zoom towards the point
    float zoomFactor = zoom_ / oldZoom;
    panX_ = x - (x - panX_) * zoomFactor;
    panY_ = y - (y - panY_) * zoomFactor;
}

void DXCanvas::PanBy(float dx, float dy) {
    panX_ += dx;
    panY_ += dy;
}

void DXCanvas::Render() {
    context_->OMSetRenderTargets(1, rtv_.GetAddressOf(), nullptr);

    D3D11_VIEWPORT vp = {};
    vp.Width = (float)width_;
    vp.Height = (float)height_;
    context_->RSSetViewports(1, &vp);

    // Update constant buffer
    struct TransformCB {
        float panX;
        float panY;
        float zoom;
        float padding1;
        float canvasWidth;
        float canvasHeight;
        float padding2[2];
    };
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(context_->Map(transformCB_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        TransformCB* cb = (TransformCB*)mapped.pData;
        cb->panX = panX_;
        cb->panY = panY_;
        cb->zoom = zoom_;
        cb->padding1 = 0.0f;
        cb->canvasWidth = (float)width_;
        cb->canvasHeight = (float)height_;
        cb->padding2[0] = 0.0f;
        cb->padding2[1] = 0.0f;
        context_->Unmap(transformCB_.Get(), 0);
    }

    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context_->VSSetShader(vs_.Get(), nullptr, 0);
    context_->PSSetShader(ps_.Get(), nullptr, 0);
    context_->VSSetConstantBuffers(0, 1, transformCB_.GetAddressOf());

    ID3D11ShaderResourceView* sr = canvasSRV_.Get();
    context_->PSSetShaderResources(0, 1, &sr);

    ID3D11SamplerState* smp = samplerState_.Get();
    context_->PSSetSamplers(0, 1, &smp);

    context_->Draw(3, 0);

    swapChain_->Present(1, 0);

    ID3D11ShaderResourceView* nullSR = nullptr;
    context_->PSSetShaderResources(0, 1, &nullSR);
}
