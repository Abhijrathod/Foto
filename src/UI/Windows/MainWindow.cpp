

// File: src/UI/Windows/MainWindow.cpp
#include "MainWindow.h"
#include <string>

namespace {
    const char* kMainClassName = "PhotoEditorMainWndClass";
}

MainWindow::MainWindow(HINSTANCE hInstance)
: hInstance_(hInstance)
{}

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

LRESULT MainWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        // create child controls: toolbar (left), canvas (center), right panels
        RECT rc; GetClientRect(hwnd_, &rc);
        int toolbarW = 64;
        int rightW = 300;

        CreateWindowExA(0, "STATIC", NULL,
            WS_CHILD | WS_VISIBLE | SS_BLACKFRAME,
            0, 0, toolbarW, rc.bottom,
            hwnd_, (HMENU)1001, hInstance_, NULL);

        CreateWindowExA(0, "STATIC", NULL,
            WS_CHILD | WS_VISIBLE | SS_BLACKFRAME,
            toolbarW, 0, rc.right - toolbarW - rightW, rc.bottom,
            hwnd_, (HMENU)1002, hInstance_, NULL);

        CreateWindowExA(0, "STATIC", NULL,
            WS_CHILD | WS_VISIBLE | SS_BLACKFRAME,
            rc.right - rightW, 0, rightW, rc.bottom,
            hwnd_, (HMENU)1003, hInstance_, NULL);
        return 0;
    }
    case WM_SIZE: {
        // resize children
        RECT rc; GetClientRect(hwnd_, &rc);
        int toolbarW = 64;
        int rightW = 300;
        HWND hLeft = GetDlgItem(hwnd_, 1001);
        HWND hCanvas = GetDlgItem(hwnd_, 1002);
        HWND hRight = GetDlgItem(hwnd_, 1003);
        if (hLeft) SetWindowPos(hLeft, NULL, 0, 0, toolbarW, rc.bottom, SWP_NOZORDER);
        if (hCanvas) SetWindowPos(hCanvas, NULL, toolbarW, 0, rc.right - toolbarW - rightW, rc.bottom, SWP_NOZORDER);
        if (hRight) SetWindowPos(hRight, NULL, rc.right - rightW, 0, rightW, rc.bottom, SWP_NOZORDER);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd_, msg, wParam, lParam);
}

