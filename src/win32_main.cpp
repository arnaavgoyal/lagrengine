#include <windows.h>

#define WINDOW_CLASS_NAME "window"
#define WINDOW_TITLE "Engine"

/**
 * Window procedure callback to handle messages
 * @param window the window to handle the message for
 * @param msg the message to handle
 * @param wParam additional message data
 * @param lParam additional message data
 * @return the result of the message processing, depends on the message
 */
LRESULT CALLBACK windowCallback(HWND window, UINT msg, WPARAM wParam,
        LPARAM lParam) {
    LRESULT result;
    switch(msg) {
        case WM_CLOSE:
            PostQuitMessage(0);
            result = 0;
            break;
        default:
            result = DefWindowProcA(window, msg, wParam, lParam);
    }

    return result;
}

/**
 * Win32 entry point
 * @param inst the current instance of the program
 * @param prevInst the previous instance of the program
 * @param cmdline the command the application was opened with
 * @param cmdShow how to chow the window (minimized, maximized, ...)
 * @return the exit code of the program
 */
int APIENTRY WinMain(HINSTANCE inst, HINSTANCE prevInst, PSTR cmdLine,
        int cmdShow) {
    // setup the window class
    WNDCLASSEXA windowClass;
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = windowCallback;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = inst;
    windowClass.hIcon = 0;
    windowClass.hCursor = 0;
    windowClass.hbrBackground = 0;
    windowClass.lpszMenuName = 0;
    windowClass.lpszClassName = WINDOW_CLASS_NAME;
    windowClass.hIconSm = 0;

    if(!RegisterClassExA(&windowClass)) {
        // TODO document failure somehow
        // Arnaav, you might be wondering, why don't you just print to stderr or
        // stdout? Well, since this is a "Window" application and not a
        // "Console" application, stderr and stdout do not exist. We would have
        // to initialize our own console to do so.

        // Windows documentation says to return 0 if message loop is not
        // reached, we can choose to disregard this
        return 0;
    }

    HWND window = CreateWindowExA(0, WINDOW_CLASS_NAME, WINDOW_TITLE,
            WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 500,
            500, 0, 0, inst, 0);

    if(!window) {
        // TODO document failure somehow
        // Reference note in the RegisterClassExA if statement.

        // Windows documentation says to return 0 if message loop is not
        // reached, we can choose to disregard this
        return 0;
    }

    bool running = true;
    MSG msg;

    while(running) {
        // Check for a message with no filters, remove it if there is one
        while(PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
            // do NOT sent the quit message to the window procedure
            if(msg.message == WM_QUIT) {
                running = false;
            }

            else {
                // decode the message then send it to the window procedure
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
        }
    }

    // Windows wants the wParam of the WM_QUIT message returned, we can choose
    // to disregard this
    return msg.wParam;
}
