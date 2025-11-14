#pragma once
#include "ToolBase.h"
#include <windows.h>
#include <cstdint>

class DXCanvas;

class BrushTool : public ToolBase {
public:
    BrushTool(DXCanvas* canvas);
    ~BrushTool();

    void OnActivate() override;
    void OnDeactivate() override;

    void OnMouseDown(const Vector2D& pos, uint32_t button) override;
    void OnMouseUp(const Vector2D& pos, uint32_t button) override;
    void OnMouseMove(const Vector2D& pos) override;

    void SetSize(float size) override { size_ = static_cast<int>(size); }
    float GetSize() const override { return static_cast<float>(size_); }
    
    void SetColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) override {
        cr_ = r; cg_ = g; cb_ = b; ca_ = a;
    }

    HCURSOR GetCursor() const override { return LoadCursor(NULL, IDC_CROSS); }

    // Legacy methods for compatibility
    void OnLButtonDown(int x, int y);
    void OnLButtonUp(int x, int y);
    void OnMouseMove(int x, int y);

private:
    DXCanvas* canvas_;
    bool painting_ = false;
    int size_ = 16;
    uint8_t cr_ = 255, cg_ = 0, cb_ = 0, ca_ = 255; // default red
    int lastX_ = -1, lastY_ = -1;

    void StrokeTo(int x, int y);
};
