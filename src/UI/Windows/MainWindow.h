#pragma once
#include <windows.h>

class MainWindow {
public:
    MainWindow(HINSTANCE hInstance);
    ~MainWindow();
    bool Create();
    void Show(int nCmdShow);
    HWND Handle() const { return hwnd_; }

private:
    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

    HINSTANCE hInstance_;
    HWND hwnd_ = nullptr;
    int width_ = 1280;
    int height_ = 800;
};
