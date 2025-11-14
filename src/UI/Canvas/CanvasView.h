#pragma once
#include <windows.h>

// Forward declarations
class DXCanvas;
class BrushTool;

HWND CreateCanvasWindow(HWND parent, HINSTANCE hInst, int id);
LRESULT CALLBACK CanvasViewProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct CanvasUserData {
    DXCanvas* dxptr = nullptr;
    BrushTool* toolptr = nullptr;
    bool panning = false;
    int panStartX = 0;
    int panStartY = 0;
    float panStartPanX = 0.0f;
    float panStartPanY = 0.0f;
};
