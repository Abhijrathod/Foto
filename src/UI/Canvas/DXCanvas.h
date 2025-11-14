#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl.h>
#include <vector>
#include <cstdint>

class DXCanvas {
public:
    DXCanvas(HWND hostWnd);
    ~DXCanvas();

    bool Initialize();
    void Resize(UINT width, UINT height);
    void Render();

    bool InitCanvasBuffer(UINT width, UINT height);

    // Upload whole texture
    void UploadCanvasToGPU();

    // Upload region
    void UploadCanvasRegion(UINT x, UINT y, UINT w, UINT h);

    // CPU brush
    void DrawCircleToCPU(int cx, int cy, int radius,
        uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    // Load image from RGBA8 buffer
    bool LoadImageFromBuffer(const uint8_t* pixels, UINT width, UINT height);

    // Get canvas dimensions
    UINT GetWidth() const { return width_; }
    UINT GetHeight() const { return height_; }

    // Zoom/Pan
    void SetZoom(float zoom) { zoom_ = zoom; }
    float GetZoom() const { return zoom_; }
    void SetPan(float x, float y) { panX_ = x; panY_ = y; }
    void GetPan(float& x, float& y) const { x = panX_; y = panY_; }
    void ZoomAtPoint(float zoom, float x, float y);
    void PanBy(float dx, float dy);

private:
    float zoom_ = 1.0f;
    float panX_ = 0.0f;
    float panY_ = 0.0f;
    HWND hostWnd_;
    UINT width_ = 0;
    UINT height_ = 0;

    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv_;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> canvasTexture_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> canvasSRV_;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vs_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> ps_;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> transformCB_;

    std::vector<uint8_t> cpuBuffer_; // RGBA8
};
