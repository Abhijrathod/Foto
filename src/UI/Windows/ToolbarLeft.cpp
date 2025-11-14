// File: src/UI/Windows/ToolbarLeft.cpp
#include "ToolbarLeft.h"
#include <commctrl.h>

void RegisterToolbarClass(HINSTANCE hInstance) {
    // Using standard button controls inside the static left pane is simpler for now.
    // For advanced rendering, you can register a custom class.
    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_WIN95_CLASSES };
    InitCommonControlsEx(&icc);
}