// File: src/main.cpp
#include <windows.h>
#include "UI/Windows/MainWindow.h"
#include <comdef.h>

#pragma comment(lib, "ole32.lib")

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    // Initialize COM for WIC
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    
    MainWindow app(hInstance);
    if (!app.Create()) {
        CoUninitialize();
        return -1;
    }
    app.Show(nCmdShow);


    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    CoUninitialize();
    return 0;
}
