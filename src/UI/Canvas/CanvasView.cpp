#define NOMINMAX
#include "CanvasView.h"
#include "../Canvas/DXCanvas.h"
#include "../../Tools/BrushTool.h"
#include <windowsx.h>

static const char* CANVAS_CLASS = "CanvasViewClass";

HWND CreateCanvasWindow(HWND parent, HINSTANCE hInst, int id)
{
    static bool registered = false;

    if (!registered) {
        WNDCLASSEXA wc = {};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = CanvasViewProc;
        wc.hInstance = hInst;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.lpszClassName = CANVAS_CLASS;
        RegisterClassExA(&wc);
        registered = true;
    }

    return CreateWindowExA(
        0, CANVAS_CLASS, "",
        WS_CHILD | WS_VISIBLE,
        0, 0, 100, 100,
        parent,
        (HMENU)id,
        hInst, nullptr
    );
}

LRESULT CALLBACK CanvasViewProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CanvasUserData* cud =
        (CanvasUserData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (msg)
    {
    case WM_CREATE: {
        // Initialize DXCanvas and BrushTool when canvas window is created
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        
        DXCanvas* dx = new DXCanvas(hwnd);
        if (!dx->Initialize()) {
            MessageBoxA(hwnd, "DXCanvas initialization failed", "Error", MB_ICONERROR);
            delete dx;
            return -1;
        }

        BrushTool* brush = new BrushTool(dx);
        
        CanvasUserData* newCud = new CanvasUserData();
        newCud->dxptr = dx;
        newCud->toolptr = brush;
        
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)newCud);
        
        // Initial render
        dx->Render();
        return 0;
    }

    case WM_DESTROY: {
        if (cud) {
            delete cud->toolptr;
            delete cud->dxptr;
            delete cud;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
        }
        return 0;
    }

    case WM_PAINT: {
        // The actual DXCanvas handles all drawing.
        // Just call Render() when Windows tells us.
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);

        if (cud && cud->dxptr) {
            cud->dxptr->Render();
        }

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_SIZE: {
        if (cud && cud->dxptr) {
            RECT rc;
            GetClientRect(hwnd, &rc);
            cud->dxptr->Resize(rc.right - rc.left, rc.bottom - rc.top);
            cud->dxptr->Render();
        }
        return 0;
    }

    case WM_LBUTTONDOWN: {
        if (cud) {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            
            // Check if spacebar is held for panning
            if (GetKeyState(VK_SPACE) & 0x8000) {
                cud->panning = true;
                cud->panStartX = x;
                cud->panStartY = y;
                if (cud->dxptr) {
                    cud->dxptr->GetPan(cud->panStartPanX, cud->panStartPanY);
                }
                SetCapture(hwnd);
            }
            else if (cud->toolptr) {
                SetCapture(hwnd);
                cud->toolptr->OnLButtonDown(x, y);
                // Render after brush stroke starts
                if (cud->dxptr) {
                    cud->dxptr->Render();
                }
            }
        }
        return 0;
    }

    case WM_MOUSEMOVE: {
        if (cud) {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            
            // Handle panning (spacebar + drag)
            if (cud->panning && (wParam & MK_LBUTTON)) {
                if (cud->dxptr) {
                    int dx_pan = x - cud->panStartX;
                    int dy_pan = y - cud->panStartY;
                    cud->dxptr->SetPan(cud->panStartPanX + dx_pan, cud->panStartPanY + dy_pan);
                    cud->dxptr->Render();
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            // Handle brush tool
            else if ((wParam & MK_LBUTTON) && cud->toolptr) {
                cud->toolptr->OnMouseMove(x, y);
                // Render after brush stroke
                if (cud->dxptr) {
                    cud->dxptr->Render();
                }
            }
        }
        return 0;
    }

    case WM_LBUTTONUP: {
        if (cud) {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            
            if (cud->panning) {
                cud->panning = false;
                ReleaseCapture();
            }
            else if (cud->toolptr) {
                cud->toolptr->OnLButtonUp(x, y);
                ReleaseCapture();
                // Final render after brush stroke ends
                if (cud->dxptr) {
                    cud->dxptr->Render();
                }
            }
        }
        return 0;
    }

    case WM_MOUSEWHEEL: {
        // Ctrl + scroll for zoom
        if (GetKeyState(VK_CONTROL) & 0x8000) {
            if (cud && cud->dxptr) {
                POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                ScreenToClient(hwnd, &pt);
                
                int cx = pt.x;
                int cy = pt.y;
                
                float delta = GET_WHEEL_DELTA_WPARAM(wParam) / 120.0f;
                float zoomFactor = 1.0f + delta * 0.1f;
                float newZoom = cud->dxptr->GetZoom() * zoomFactor;
                
                cud->dxptr->ZoomAtPoint(newZoom, (float)cx, (float)cy);
                cud->dxptr->Render();
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        break;
    }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
