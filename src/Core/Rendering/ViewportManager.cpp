#include "ViewportManager.h"

ViewportManager::ViewportManager() {
}

ViewportManager::~ViewportManager() {
}

void ViewportManager::SetViewportSize(uint32_t width, uint32_t height) {
    viewportWidth_ = width;
    viewportHeight_ = height;
}

void ViewportManager::SetCanvasSize(uint32_t width, uint32_t height) {
    canvasWidth_ = width;
    canvasHeight_ = height;
}

void ViewportManager::SetZoom(float zoom) {
    zoom_ = std::max(0.1f, std::min(10.0f, zoom));
}

void ViewportManager::SetPan(const Vector2D& pan) {
    pan_ = pan;
}

void ViewportManager::ZoomAtPoint(float zoom, const Vector2D& point) {
    float oldZoom = zoom_;
    SetZoom(zoom);
    
    float zoomFactor = zoom_ / oldZoom;
    Vector2D offset = point - pan_;
    pan_ = point - offset * zoomFactor;
}

void ViewportManager::PanBy(const Vector2D& delta) {
    pan_ += delta;
}

Vector2D ViewportManager::ScreenToCanvas(const Vector2D& screenPos) const {
    Vector2D normalized = Vector2D(
        screenPos.x / viewportWidth_,
        screenPos.y / viewportHeight_
    );
    return (normalized - Vector2D(0.5f, 0.5f)) * (1.0f / zoom_) + pan_ + Vector2D(canvasWidth_ * 0.5f, canvasHeight_ * 0.5f);
}

Vector2D ViewportManager::CanvasToScreen(const Vector2D& canvasPos) const {
    Vector2D offset = canvasPos - Vector2D(canvasWidth_ * 0.5f, canvasHeight_ * 0.5f) - pan_;
    Vector2D normalized = offset * zoom_ + Vector2D(0.5f, 0.5f);
    return Vector2D(normalized.x * viewportWidth_, normalized.y * viewportHeight_);
}

Matrix ViewportManager::GetTransformMatrix() const {
    return Matrix::Translation(pan_.x, pan_.y) * Matrix::Scale(zoom_) * Matrix::Translation(-canvasWidth_ * 0.5f, -canvasHeight_ * 0.5f);
}

