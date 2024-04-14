#include <thread>
#include <cassert>
#include <stdio.h>
#include <iostream>
#include <windows.h>
#include <fcntl.h>
#include <io.h>

#include <glad/gl.h>

#include "engine.h"

#define WINDOW_CLASS_NAME "window"
#define WINDOW_TITLE "Lagrengine"

// OpenGL DLL
HMODULE opengl;

/**
 * Used by GLAD to get OpenGL functions from the OpenGL dll
 * @param name the name of the function to get
 * @return a pointer to the function
 */
GLADapiproc getOpenGLProc(char const *name) {
    return (GLADapiproc) GetProcAddress(opengl, name);
}

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
        case WM_DESTROY:
            PostQuitMessage(0);
            result = 0;
            break;
        default:
            result = DefWindowProcA(window, msg, wParam, lParam);
    }

    return result;
}

/**
 * Sets up the C and C++ runtime io (file descriptors
 * and iostreams, respectively).
 * Microsoft is a terrible company, so we need this.
 */
void setupCRTIO() {

    // get the stream handles
    HANDLE winStdinHandle = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE winStdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE winStderrHandle = GetStdHandle(STD_ERROR_HANDLE);
    assert(
        winStdinHandle != INVALID_HANDLE_VALUE
        && winStdoutHandle != INVALID_HANDLE_VALUE
        && winStderrHandle != INVALID_HANDLE_VALUE
    );

    // set the crt streams to these streams...
    FILE *fp = nullptr;

    // stdin
    if (!freopen_s(&fp, "CONIN$", "r", stdin)) {
        setvbuf(stdin, nullptr, _IONBF, 0);
    }

    // stdout
    if (!freopen_s(&fp, "CONOUT$", "w", stdout)) {
        setvbuf(stdout, nullptr, _IONBF, 0);
    }

    // stderr
    if (!freopen_s(&fp, "CONOUT$", "w", stderr)) {
        setvbuf(stderr, nullptr, _IONBF, 0);
    }

    // sync c++ streams
    std::ios::sync_with_stdio(true);
    std::cin.clear();
    std::cout.clear();
    std::cerr.clear();
}

HWND createWindow(HINSTANCE inst, char const *title, int width, int height) {
    // setup the window class
    WNDCLASSEXA window_class;
    window_class.cbSize = sizeof(WNDCLASSEX);
    window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = windowCallback;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = inst;
    window_class.hIcon = 0;
    window_class.hCursor = 0;
    window_class.hbrBackground = 0;
    window_class.lpszMenuName = 0;
    window_class.lpszClassName = WINDOW_CLASS_NAME;
    window_class.hIconSm = 0;

    // register the class
    if(!RegisterClassExA(&window_class)) {
        // TODO is this the best way to show the error? Or should we log file?
        fprintf(stderr, "Failed to register window class: %s\n",
                WINDOW_CLASS_NAME);

        // NULL on failure
        return 0;
    }

    // we want to specify the size of the window in terms of the client rect,
    // not the total dimensions like windows does it
    RECT client_rect = {
        .right = width,
        .bottom = height,
    };

    AdjustWindowRect(&client_rect, WS_OVERLAPPEDWINDOW, false);

    // create the window
    HWND window = CreateWindowExA(0, WINDOW_CLASS_NAME, title,
            WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
            client_rect.right - client_rect.left,
            client_rect.bottom - client_rect.top, 0, 0, inst, 0);

    if(!window) {
        // TODO is this the best way to show the error? Or should we log file?
        fprintf(stderr, "Failed to create window: %s\n", WINDOW_TITLE);

        // NULL on failure
        return 0;
    }

    return window;
}

/**
 * Setup an OpenGL context
 */
int setupOpenGL(HDC device_context) {
    PIXELFORMATDESCRIPTOR pfd = {
        .nSize = sizeof(PIXELFORMATDESCRIPTOR),
        .nVersion = 1,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .iPixelType = PFD_TYPE_RGBA,
        .cColorBits = 32,
        .cDepthBits = 24,
        .cStencilBits = 8,
        .cAuxBuffers = 0,
        .iLayerType = PFD_MAIN_PLANE,
    };

    int pf_idx = ChoosePixelFormat(device_context, &pfd);

    if(!pf_idx) {
        // TODO is this the best way to show the error? Or should we log file?
        fprintf(stderr, "Failed to choose pixel format for the given DC\n");

        // NULL on failure
        return 0;
    }

    if(!SetPixelFormat(device_context, pf_idx, &pfd)) {
        // TODO is this the best way to show the error? Or should we log file?
        fprintf(stderr, "Failed to set pixel format for the given DC\n");

        // NULL on failure
        return 0;
    }

    HGLRC opengl_context = wglCreateContext(device_context);
    if(!opengl_context) {
        fprintf(stderr, "Failed to create an OpenGL context for the given DC");

        // NULL on failure
        return 0;
    }

    if(!wglMakeCurrent(device_context, opengl_context)) {
        fprintf(stderr, "Failed to make the OpenGL context current");

        // NULL on failure
        return 0;
    }

    return 1;
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

    // attempt to get a console...
    if (!AllocConsole()) {
        // welp, windows sucks
        return 0;
    }

    // yay, we have a console!
    // setup the crt io
    setupCRTIO();

    // start up the engine thread
    std::thread engine(engineInit);

    // create a window
    HWND window = createWindow(inst, WINDOW_TITLE, 600, 300);

    if(!window) {
        fprintf(stderr, "Could not create window %s, aborting\n", WINDOW_TITLE);

        // Windows wants 0 returned if message loop is not reached
        return 0;
    }

    // opengl setup
    HDC device_context = GetDC(window);
    if(!device_context) {
        fprintf(stderr, "Could not get device context for window %s, aborting",
                WINDOW_TITLE);

        // Windows wants 0 returned if message loop is not reached
        return 0;
    }

    setupOpenGL(device_context);

    opengl = LoadLibraryA("opengl32.dll");

    if(!opengl) {
        // TODO is this the best way to show the error? Or should we log file?
        fprintf(stderr, "Could not find opengl32.dll: %s\n", WINDOW_TITLE);

        // Windows documentation says to return 0 if message loop is not
        // reached, we can choose to disregard this
        return 0;
    }

    if(!gladLoadGLUserPtr((GLADuserptrloadfunc) GetProcAddress, opengl)) {
        // TODO is this the best way to show the error? Or should we log file?
        fprintf(stderr, "Failed to initialize OpenGL: %s\n", WINDOW_TITLE);

        // Windows documentation says to return 0 if message loop is not
        // reached, we can choose to disregard this
        return 0;
    }

    bool running = true;
    MSG msg;

    glViewport(0, 0, 600, 300);

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
		glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

        SwapBuffers(device_context);
    }

    // Windows wants the wParam of the WM_QUIT message returned, we can choose
    // to disregard this
    return msg.wParam;
}
