// File: src/main.cpp
#include <windows.h>
#include "UI/Windows/MainWindow.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
MainWindow app(hInstance);
if (!app.Create()) return -1;
app.Show(nCmdShow);


MSG msg = {};
while (GetMessage(&msg, NULL, 0, 0) > 0) {
TranslateMessage(&msg);
DispatchMessage(&msg);
}
return 0;
}
