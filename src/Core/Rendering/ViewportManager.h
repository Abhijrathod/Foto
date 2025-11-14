#pragma once
#include "../Math/Matrix.h"
#include "../Math/Vector2D.h"
#include <cstdint>

class ViewportManager {
public:
    ViewportManager();
    ~ViewportManager();

    void SetViewportSize(uint32_t width, uint32_t height);
    void SetCanvasSize(uint32_t width, uint32_t height);
    
    void SetZoom(float zoom);
    float GetZoom() const { return zoom_; }
    
    void SetPan(const Vector2D& pan);
    Vector2D GetPan() const { return pan_; }
    
    void ZoomAtPoint(float zoom, const Vector2D& point);
    void PanBy(const Vector2D& delta);
    
    Vector2D ScreenToCanvas(const Vector2D& screenPos) const;
    Vector2D CanvasToScreen(const Vector2D& canvasPos) const;
    
    Matrix GetTransformMatrix() const;

private:
    uint32_t viewportWidth_ = 0;
    uint32_t viewportHeight_ = 0;
    uint32_t canvasWidth_ = 0;
    uint32_t canvasHeight_ = 0;
    float zoom_ = 1.0f;
    Vector2D pan_;
};

