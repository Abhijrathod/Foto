#define NOMINMAX
#include "LayerPanel.h"
#include <algorithm>
#include <commctrl.h>

namespace {
    const char* kLayerPanelClassName = "LayerPanelClass";
    const int kLayerHeight = 60;
    const int kThumbnailSize = 48;
}

LayerPanel::LayerPanel(HWND parent, HINSTANCE hInstance)
    : parent_(parent), hInstance_(hInstance) {
    
    // Register window class
    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance_;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = kLayerPanelClassName;
    RegisterClassExA(&wc);
}

LayerPanel::~LayerPanel() {
    for (auto& layer : layers_) {
        if (layer.textureData) {
            delete[] (uint8_t*)layer.textureData;
        }
    }
}

void LayerPanel::Create(int x, int y, int width, int height) {
    hwnd_ = CreateWindowExA(
        0,
        kLayerPanelClassName,
        "Layers",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL,
        x, y, width, height,
        parent_, (HMENU)3001, hInstance_, this);
    
    SetWindowLongPtr(hwnd_, GWLP_USERDATA, (LONG_PTR)this);
}

void LayerPanel::AddLayer(const std::string& name) {
    LayerPanelItem layer;
    layer.name = name.empty() ? ("Layer " + std::to_string((int)layers_.size() + 1)) : name;
    layer.visible = true;
    layer.opacity = 1.0f;
    layers_.push_back(layer);
    selectedLayer_ = (int)layers_.size() - 1;
    InvalidateRect(hwnd_, NULL, TRUE);
}

void LayerPanel::DeleteLayer(int index) {
    if (index >= 0 && index < (int)layers_.size()) {
        if (layers_[index].textureData) {
            delete[] (uint8_t*)layers_[index].textureData;
        }
        layers_.erase(layers_.begin() + index);
        if (selectedLayer_ >= (int)layers_.size()) {
            selectedLayer_ = (int)layers_.size() - 1;
        }
        InvalidateRect(hwnd_, NULL, TRUE);
    }
}

void LayerPanel::SetLayerVisible(int index, bool visible) {
    if (index >= 0 && index < (int)layers_.size()) {
        layers_[index].visible = visible;
        InvalidateRect(hwnd_, NULL, TRUE);
    }
}

void LayerPanel::UpdateThumbnail(int index, const uint8_t* pixels, UINT width, UINT height) {
    if (index < 0 || index >= (int)layers_.size() || !pixels) return;
    
    LayerPanelItem& layer = layers_[index];
    if (layer.textureData) {
        delete[] (uint8_t*)layer.textureData;
    }
    
    // Create thumbnail (scaled down)
    UINT thumbW = std::min((UINT)kThumbnailSize, width);
    UINT thumbH = std::min((UINT)kThumbnailSize, height);
    layer.width = thumbW;
    layer.height = thumbH;
    
    uint8_t* thumb = new uint8_t[thumbW * thumbH * 4];
    float scaleX = (float)width / thumbW;
    float scaleY = (float)height / thumbH;
    
    for (UINT y = 0; y < thumbH; y++) {
        for (UINT x = 0; x < thumbW; x++) {
            UINT srcX = (UINT)(x * scaleX);
            UINT srcY = (UINT)(y * scaleY);
            UINT srcIdx = (srcY * width + srcX) * 4;
            UINT dstIdx = (y * thumbW + x) * 4;
            
            thumb[dstIdx + 0] = pixels[srcIdx + 0];
            thumb[dstIdx + 1] = pixels[srcIdx + 1];
            thumb[dstIdx + 2] = pixels[srcIdx + 2];
            thumb[dstIdx + 3] = pixels[srcIdx + 3];
        }
    }
    
    layer.textureData = thumb;
    InvalidateRect(hwnd_, NULL, TRUE);
}

LayerPanelItem* LayerPanel::GetLayer(int index) {
    if (index >= 0 && index < (int)layers_.size()) {
        return &layers_[index];
    }
    return nullptr;
}

void LayerPanel::OnPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd_, &ps);
    
    RECT rc;
    GetClientRect(hwnd_, &rc);
    
    // Dark background
    HBRUSH bgBrush = CreateSolidBrush(RGB(45, 45, 45));
    FillRect(hdc, &rc, bgBrush);
    DeleteObject(bgBrush);
    
    // Draw layers
    int y = 4 - scrollY_;
    for (size_t i = 0; i < layers_.size(); i++) {
        int layerY = y + (int)i * kLayerHeight;
        if (layerY + kLayerHeight < 0 || layerY > rc.bottom) continue;
        
        RECT layerRc = { 4, layerY, rc.right - 4, layerY + kLayerHeight - 2 };
        
        // Selected highlight
        if ((int)i == selectedLayer_) {
            HBRUSH selBrush = CreateSolidBrush(RGB(60, 90, 140));
            FillRect(hdc, &layerRc, selBrush);
            DeleteObject(selBrush);
        }
        else {
            HBRUSH layerBrush = CreateSolidBrush(RGB(55, 55, 55));
            FillRect(hdc, &layerRc, layerBrush);
            DeleteObject(layerBrush);
        }
        
        // Thumbnail
        if (layers_[i].textureData && layers_[i].width > 0 && layers_[i].height > 0) {
            // Simple bitmap drawing (would use proper bitmap in production)
            RECT thumbRc = { layerRc.left + 4, layerRc.top + 4, 
                           layerRc.left + 4 + kThumbnailSize, layerRc.top + 4 + kThumbnailSize };
            HBRUSH thumbBg = CreateSolidBrush(RGB(100, 100, 100));
            FillRect(hdc, &thumbRc, thumbBg);
            DeleteObject(thumbBg);
        }
        
        // Visibility toggle
        RECT visRc = { layerRc.right - 40, layerRc.top + 20, layerRc.right - 20, layerRc.top + 40 };
        if (layers_[i].visible) {
            HBRUSH visBrush = CreateSolidBrush(RGB(100, 200, 100));
            FillRect(hdc, &visRc, visBrush);
            DeleteObject(visBrush);
        }
        else {
            HBRUSH visBrush = CreateSolidBrush(RGB(100, 100, 100));
            FillRect(hdc, &visRc, visBrush);
            DeleteObject(visBrush);
        }
        
        // Layer name
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));
        RECT textRc = { layerRc.left + kThumbnailSize + 12, layerRc.top + 8, 
                       layerRc.right - 50, layerRc.bottom };
        DrawTextA(hdc, layers_[i].name.c_str(), -1, &textRc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }
    
    EndPaint(hwnd_, &ps);
}

void LayerPanel::OnLButtonDown(int x, int y) {
    int layerIndex = (y + scrollY_ - 4) / kLayerHeight;
    if (layerIndex >= 0 && layerIndex < (int)layers_.size()) {
        selectedLayer_ = layerIndex;
        
        // Check if clicked on visibility toggle
        RECT layerRc = { 4, layerIndex * kLayerHeight - scrollY_ + 4, 
                       0, (layerIndex + 1) * kLayerHeight - scrollY_ + 4 };
        GetClientRect(hwnd_, &layerRc);
        layerRc.right = layerRc.right - 4;
        layerRc.left = layerRc.right - 40;
        layerRc.top = layerIndex * kLayerHeight - scrollY_ + 24;
        layerRc.bottom = layerRc.top + 20;
        
        if (x >= layerRc.left && x <= layerRc.right && 
            y >= layerRc.top && y <= layerRc.bottom) {
            SetLayerVisible(layerIndex, !layers_[layerIndex].visible);
        }
        
        InvalidateRect(hwnd_, NULL, TRUE);
    }
}

LRESULT CALLBACK LayerPanel::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    LayerPanel* self = (LayerPanel*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (msg) {
    case WM_PAINT:
        if (self) self->OnPaint();
        return 0;
        
    case WM_LBUTTONDOWN:
        if (self) {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            self->OnLButtonDown(x, y);
        }
        return 0;
        
    case WM_VSCROLL:
        if (self) {
            int delta = 0;
            switch (LOWORD(wParam)) {
            case SB_LINEDOWN: delta = 20; break;
            case SB_LINEUP: delta = -20; break;
            case SB_PAGEDOWN: delta = 200; break;
            case SB_PAGEUP: delta = -200; break;
            }
            self->scrollY_ = std::max(0, self->scrollY_ + delta);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void LayerPanel::Render() {
    InvalidateRect(hwnd_, NULL, TRUE);
}

