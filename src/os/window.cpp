#include <cstdio>

#include "os/window.h"

bool registerWindowClass(HINSTANCE inst, char const *class_name,
    LRESULT (CALLBACK *windowCallback)(HWND, UINT, WPARAM, LPARAM)) {

    // setup the window class
    WNDCLASSEXA window_class = {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = windowCallback,
        .hInstance = inst,
        .lpszClassName = class_name,
    };

    // register the class
    if(!RegisterClassExA(&window_class)) {
        fprintf(stderr, "Failed to register window class: %s\n", class_name);

        // false on failure
        return false;
    }

    return true;
}

HWND createWindow(HINSTANCE inst, char const *class_name, char const *title,
        int width, int height, bool visible) {
            
    // we want to specify the size of the window in terms of the client rect,
    // not the total dimensions like windows does it
    RECT client_rect = {
        .right = width,
        .bottom = height,
    };

    AdjustWindowRect(&client_rect, WS_OVERLAPPEDWINDOW, false);

    DWORD style = (visible ? WS_OVERLAPPEDWINDOW | WS_VISIBLE
            : WS_OVERLAPPEDWINDOW);

    // create the window
    HWND window = CreateWindowExA(0, class_name, title, style, CW_USEDEFAULT,
            CW_USEDEFAULT, client_rect.right - client_rect.left,
            client_rect.bottom - client_rect.top, 0, 0, inst, 0);

    if(!window) {
        fprintf(stderr, "Failed to create window: %s\n", title);

        // NULL on failure
        return 0;
    }

    return window;
}
