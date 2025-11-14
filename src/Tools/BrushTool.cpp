#define NOMINMAX
#include <windows.h>

#include "BrushTool.h"
#include "../UI/Canvas/DXCanvas.h"
#include <algorithm>
#include <cmath>

BrushTool::BrushTool(DXCanvas* canvas) : ToolBase("Brush"), canvas_(canvas) {}

BrushTool::~BrushTool() {}

void BrushTool::OnActivate() {
    painting_ = false;
}

void BrushTool::OnDeactivate() {
    painting_ = false;
}

void BrushTool::OnMouseDown(const Vector2D& pos, uint32_t button) {
    if (button != 0) return; // Only handle left button
    painting_ = true;
    lastX_ = static_cast<int>(pos.x);
    lastY_ = static_cast<int>(pos.y);
    StrokeTo(lastX_, lastY_);
}

void BrushTool::OnMouseUp(const Vector2D& pos, uint32_t button) {
    if (button != 0) return;
    if (painting_) {
        StrokeTo(static_cast<int>(pos.x), static_cast<int>(pos.y));
    }
    painting_ = false;
    lastX_ = -1; lastY_ = -1;
}

void BrushTool::OnMouseMove(const Vector2D& pos) {
    if (!painting_) return;
    // simple linear interpolation between last and current to avoid holes
    if (lastX_ < 0) { lastX_ = static_cast<int>(pos.x); lastY_ = static_cast<int>(pos.y); }
    int dx = static_cast<int>(pos.x) - lastX_;
    int dy = static_cast<int>(pos.y) - lastY_;
    float dist = std::sqrt(float(dx*dx + dy*dy));
    int steps = std::max(1, int(dist / (size_ * 0.25f)));
    for (int i = 1; i <= steps; ++i) {
        float t = i / float(steps);
        int ix = int(lastX_ + dx * t + 0.5f);
        int iy = int(lastY_ + dy * t + 0.5f);
        StrokeTo(ix, iy);
    }
    lastX_ = static_cast<int>(pos.x);
    lastY_ = static_cast<int>(pos.y);
}

void BrushTool::StrokeTo(int x, int y) {
    if (!canvas_) return;
    canvas_->DrawCircleToCPU(x, y, size_, cr_, cg_, cb_, ca_);
    // DXCanvas::DrawCircleToCPU calls UploadCanvasToGPU internally so we don't need to call it here
}

// Legacy methods for compatibility
void BrushTool::OnLButtonDown(int x, int y) {
    OnMouseDown(Vector2D(static_cast<float>(x), static_cast<float>(y)), 0);
}

void BrushTool::OnLButtonUp(int x, int y) {
    OnMouseUp(Vector2D(static_cast<float>(x), static_cast<float>(y)), 0);
}

void BrushTool::OnMouseMove(int x, int y) {
    // Duplicate logic to avoid recursion (can't call OnMouseMove(Vector2D) from here)
    if (!painting_) return;
    // simple linear interpolation between last and current to avoid holes
    if (lastX_ < 0) { lastX_ = x; lastY_ = y; }
    int dx = x - lastX_;
    int dy = y - lastY_;
    float dist = std::sqrt(float(dx*dx + dy*dy));
    int steps = std::max(1, int(dist / (size_ * 0.25f)));
    for (int i = 1; i <= steps; ++i) {
        float t = i / float(steps);
        int ix = int(lastX_ + dx * t + 0.5f);
        int iy = int(lastY_ + dy * t + 0.5f);
        StrokeTo(ix, iy);
    }
    lastX_ = x;
    lastY_ = y;
}
