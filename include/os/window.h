#ifndef OS_WINDOW_H
#define OS_WINDOW_H

#include <windows.h>

/**
 * Registers the window class. Notably this is different than window creation
 * now since we need to actually create two windows and destroy one of them to
 * initialize OpenGL properly. Windows is the best programming platform :)
 * @return success or failure
 */
bool registerWindowClass(HINSTANCE inst, char const *class_name,
    LRESULT (CALLBACK *windowCallback)(HWND, UINT, WPARAM, LPARAM));

/**
 * Creates a window
 * @param inst the program instance
 * @param title the title of the window
 * @param width the width of the client window
 * @param height the height of the client window
 * @param visible whether or not the window should be visible
 * @return the created window, or NULL on failure
 */
HWND createWindow(HINSTANCE inst, char const *class_name, char const *title,
    int width, int height, bool visible);

#endif
