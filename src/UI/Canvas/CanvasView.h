
// File: src/UI/Canvas/CanvasView.h
#pragma once
#include <windows.h>

LRESULT CALLBACK CanvasWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// File: src/UI/Canvas/CanvasView.cpp
#include "CanvasView.h"
#include <string>

LRESULT CALLBACK CanvasWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps);
        RECT r; GetClientRect(hwnd, &r);
        HBRUSH brush = CreateSolidBrush(RGB(43, 43, 43));
        FillRect(hdc, &r, brush);
        DeleteObject(brush);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_LBUTTONDOWN:
    case WM_MOUSEMOVE:
    case WM_LBUTTONUP:
        // future: handle drawing
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}




