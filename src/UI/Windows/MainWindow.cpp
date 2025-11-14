// File: src/UI/Windows/MainWindow.cpp
#include "MainWindow.h"

// ADD DXCanvas include
#include "UI/Canvas/DXCanvas.h"


#include <string>

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

LRESULT MainWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    // ------------------------------------------------------------
    // CREATE UI LAYOUT + INIT DIRECTX CANVAS
    // ------------------------------------------------------------
    case WM_CREATE: {
        RECT rc; GetClientRect(hwnd_, &rc);
        int toolbarW = 64;
        int rightW = 300;

        // LEFT TOOLBAR
        CreateWindowExA(0, "STATIC", NULL,
            WS_CHILD | WS_VISIBLE | SS_BLACKFRAME,
            0, 0, toolbarW, rc.bottom,
            hwnd_, (HMENU)1001, hInstance_, NULL);

        // CENTER CANVAS WINDOW
        HWND hCanvas = CreateWindowExA(
            0, "STATIC", "",
            WS_CHILD | WS_VISIBLE,
            toolbarW, 0,
            rc.right - toolbarW - rightW,
            rc.bottom,
            hwnd_, (HMENU)1002, hInstance_, NULL
        );

        // INIT DX CANVAS
        DXCanvas* dx = new DXCanvas(hCanvas);
        if (!dx->Initialize()) {
            delete dx; // fallback if needed
        } else {
            SetWindowLongPtr(hCanvas, GWLP_USERDATA, (LONG_PTR)dx);
        }

        // RIGHT PANELS
        CreateWindowExA(0, "STATIC", NULL,
            WS_CHILD | WS_VISIBLE | SS_BLACKFRAME,
            rc.right - rightW, 0, rightW, rc.bottom,
            hwnd_, (HMENU)1003, hInstance_, NULL);

        return 0;
    }

    // ------------------------------------------------------------
    // RESIZE DIRECTX CANVAS
    // ------------------------------------------------------------
    case WM_SIZE: {
        RECT rc; GetClientRect(hwnd_, &rc);
        int toolbarW = 64;
        int rightW = 300;

        HWND hLeft = GetDlgItem(hwnd_, 1001);
        HWND hCanvas = GetDlgItem(hwnd_, 1002);
        HWND hRight = GetDlgItem(hwnd_, 1003);

        if (hLeft) SetWindowPos(hLeft, NULL, 0, 0, toolbarW, rc.bottom, SWP_NOZORDER);
        if (hCanvas) SetWindowPos(hCanvas, NULL, toolbarW, 0,
            rc.right - toolbarW - rightW, rc.bottom, SWP_NOZORDER);
        if (hRight) SetWindowPos(hRight, NULL,
            rc.right - rightW, 0, rightW, rc.bottom, SWP_NOZORDER);

        // Resize DX swap chain
        if (hCanvas) {
            DXCanvas* dx = reinterpret_cast<DXCanvas*>(GetWindowLongPtr(hCanvas, GWLP_USERDATA));
            if (dx) {
                RECT crc; GetClientRect(hCanvas, &crc);
                dx->Resize(crc.right - crc.left, crc.bottom - crc.top);
                dx->Render(); // refresh
            }
        }

        return 0;
    }

    // ------------------------------------------------------------
    // RENDER DIRECTX CANVAS WHEN PAINTING
    // ------------------------------------------------------------
    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hwnd_, &ps);

        HWND hCanvas = GetDlgItem(hwnd_, 1002);
        if (hCanvas) {
            DXCanvas* dx = reinterpret_cast<DXCanvas*>(GetWindowLongPtr(hCanvas, GWLP_USERDATA));
            if (dx) dx->Render();
        }

        EndPaint(hwnd_, &ps);
        return 0;
    }

    // ------------------------------------------------------------
    // FREE GPU MEMORY
    // ------------------------------------------------------------
    case WM_DESTROY: {
        HWND hCanvas = GetDlgItem(hwnd_, 1002);
        if (hCanvas) {
            DXCanvas* dx = reinterpret_cast<DXCanvas*>(GetWindowLongPtr(hCanvas, GWLP_USERDATA));
            delete dx;
        }
        PostQuitMessage(0);
        return 0;
    }
    }

    return DefWindowProc(hwnd_, msg, wParam, lParam);
}
