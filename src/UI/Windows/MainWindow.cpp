// File: src/UI/Windows/MainWindow.cpp

#include "MainWindow.h"
#include "UI/Canvas/DXCanvas.h"
#include "UI/Canvas/CanvasView.h"
#include "Tools/BrushTool.h"
#include "Core/FileIO/ImageCodecs.h"
#include "UI/Windows/LayerPanel.h"
#include <commdlg.h>
#include <string>
#include <windowsx.h>

namespace {
    const char* kMainClassName = "PhotoEditorMainWndClass";
}

MainWindow::MainWindow(HINSTANCE hInstance)
    : hInstance_(hInstance) {}

MainWindow::~MainWindow() {
    if (hwnd_) DestroyWindow(hwnd_);
}

bool MainWindow::Create() {
    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = StaticWndProc;
    wc.hInstance = hInstance_;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = kMainClassName;

    if (!RegisterClassExA(&wc)) {
        DWORD err = GetLastError();
        (void)err;
        return false;
    }

    hwnd_ = CreateWindowExA(
        0,
        kMainClassName,
        "PhotoEditor - C++ Photoshop Clone",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        width_, height_,
        NULL, NULL, hInstance_, this);

    return hwnd_ != NULL;
}

void MainWindow::Show(int nCmdShow) {
    ShowWindow(hwnd_, nCmdShow);
    UpdateWindow(hwnd_);
}

LRESULT CALLBACK MainWindow::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCCREATE) {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        MainWindow* self = reinterpret_cast<MainWindow*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)self);
        self->hwnd_ = hwnd;
    }

    MainWindow* self = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (self) return self->WndProc(msg, wParam, lParam);
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

//
// -------------------- WNDPROC ---------------------
//
LRESULT MainWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_CREATE: {
        // LEFT TOOLBAR with darker theme
        RECT rc; GetClientRect(hwnd_, &rc);
        int toolbarW = 64;
        int rightW = 300;

        HWND hToolbar = CreateWindowExA(0, "STATIC", NULL,
            WS_CHILD | WS_VISIBLE | SS_BLACKFRAME,
            0, 0, toolbarW, rc.bottom,
            hwnd_, (HMENU)1001, hInstance_, NULL);
        
        // Set darker background for toolbar
        SetClassLongPtr(hToolbar, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(40, 40, 40)));
        
        // Add toolbar buttons (Brush tool, etc.)
        CreateWindowExA(0, "BUTTON", "B",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            8, 8, 48, 48,
            hToolbar, (HMENU)2001, hInstance_, NULL);

        // CANVAS CHILD WINDOW (center) - custom DirectX window
        // DXCanvas and BrushTool will be created in the canvas window's WM_CREATE
        HWND hCanvas = CreateCanvasWindow(hwnd_, hInstance_, 1002);
        SetWindowPos(hCanvas, NULL, toolbarW, 0, rc.right - toolbarW - rightW, rc.bottom, SWP_NOZORDER);

        // RIGHT PANELS - darker theme
        HWND hRightPanel = CreateWindowExA(0, "STATIC", NULL,
            WS_CHILD | WS_VISIBLE | SS_BLACKFRAME,
            rc.right - rightW, 0, rightW, rc.bottom,
            hwnd_, (HMENU)1003, hInstance_, NULL);
        SetClassLongPtr(hRightPanel, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(45, 45, 45)));
        
        // Create Layers Panel
        layerPanel_ = new LayerPanel(hRightPanel, hInstance_);
        layerPanel_->Create(0, 0, rightW, rc.bottom);
        layerPanel_->AddLayer("Background");

        return 0;
    }

    case WM_SIZE: {
        int toolbarW = 64;
        int rightW = 300;

        RECT rc; GetClientRect(hwnd_, &rc);
        HWND hLeft = GetDlgItem(hwnd_, 1001);
        HWND hCanvas = GetDlgItem(hwnd_, 1002);
        HWND hRight = GetDlgItem(hwnd_, 1003);

        if (hLeft)
            SetWindowPos(hLeft, NULL, 0, 0, toolbarW, rc.bottom, SWP_NOZORDER);

        if (hCanvas) {
            SetWindowPos(hCanvas, NULL, toolbarW, 0, rc.right - toolbarW - rightW, rc.bottom, SWP_NOZORDER);
            // Canvas window will handle its own WM_SIZE and resize DXCanvas
        }

        if (hRight)
            SetWindowPos(hRight, NULL, rc.right - rightW, 0, rightW, rc.bottom, SWP_NOZORDER);

        return 0;
    }

    case WM_COMMAND: {
        if (LOWORD(wParam) == ID_FILE_OPEN || (LOWORD(wParam) == 'O' && GetKeyState(VK_CONTROL) & 0x8000)) {
            // File Open Dialog
            OPENFILENAMEW ofn = {};
            wchar_t szFile[260] = {};
            ofn.lStructSize = sizeof(OPENFILENAMEW);
            ofn.hwndOwner = hwnd_;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
            ofn.lpstrFilter = L"Image Files\0*.png;*.jpg;*.jpeg;*.bmp\0All Files\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileNameW(&ofn)) {
                HWND hCanvas = GetDlgItem(hwnd_, 1002);
                if (hCanvas) {
                    CanvasUserData* cud = (CanvasUserData*)GetWindowLongPtr(hCanvas, GWLP_USERDATA);
                    if (cud && cud->dxptr) {
                        ImageData img;
                        if (ImageCodecs::LoadImage(szFile, img) && img.valid) {
                            if (cud->dxptr->LoadImageFromBuffer(img.pixels.data(), img.width, img.height)) {
                                cud->dxptr->Render();
                                InvalidateRect(hCanvas, NULL, FALSE);
                            }
                        }
                    }
                }
            }
            return 0;
        }
        break;
    }

    case WM_KEYDOWN: {
        // Ctrl+O for open
        if (wParam == 'O' && (GetKeyState(VK_CONTROL) & 0x8000)) {
            SendMessage(hwnd_, WM_COMMAND, ID_FILE_OPEN, 0);
            return 0;
        }
        break;
    }

    // Mouse events are now handled by the canvas window's WndProc (CanvasViewProc)
    // No need to handle them here since the canvas is a custom window class

    //
    // Destroy Main Window
    // Note: Canvas window will clean up DXCanvas and BrushTool in its own WM_DESTROY
    //
    case WM_DESTROY: {
        if (layerPanel_) {
            delete layerPanel_;
        }
        PostQuitMessage(0);
        return 0;
    }

    }

    return DefWindowProc(hwnd_, msg, wParam, lParam);
}
