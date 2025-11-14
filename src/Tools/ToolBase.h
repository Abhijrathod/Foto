#pragma once
#include "../Core/Math/Vector2D.h"
#include <cstdint>
#include <string>
#include <windows.h>

class CanvasView;
class Layer;

// Base class for all editing tools
class ToolBase {
public:
    ToolBase(const std::string& name);
    virtual ~ToolBase() = default;

    const std::string& GetName() const { return name_; }
    
    // Tool lifecycle
    virtual void OnActivate() {}
    virtual void OnDeactivate() {}
    
    // Mouse events (in canvas coordinates)
    virtual void OnMouseDown(const Vector2D& pos, uint32_t button) {}
    virtual void OnMouseMove(const Vector2D& pos) {}
    virtual void OnMouseUp(const Vector2D& pos, uint32_t button) {}
    virtual void OnMouseWheel(const Vector2D& pos, float delta) {}
    
    // Keyboard events
    virtual void OnKeyDown(uint32_t key) {}
    virtual void OnKeyUp(uint32_t key) {}
    
    // Tool settings
    virtual void SetSize(float size) {}
    virtual float GetSize() const { return 1.0f; }
    
    virtual void SetColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {}
    
    // Cursor
    virtual HCURSOR GetCursor() const { return LoadCursor(NULL, IDC_ARROW); }

protected:
    std::string name_;
};

