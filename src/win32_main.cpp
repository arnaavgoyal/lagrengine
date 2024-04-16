#include <thread>
#include <cassert>
#include <stdio.h>
#include <iostream>
#include <windows.h>
#include <fcntl.h>
#include <io.h>

#include <glad/gl.h>

#include "engine.h"
#include "utils/shader.h"
#include "utils/event.h"

#define WINDOW_CLASS_NAME "window"
#define WINDOW_TITLE "Lagrengine"
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 300

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
            event::trigger(WindowCloseRequestedEvent{});
            PostQuitMessage(0);
            result = 0;
            break;
        case WM_DESTROY:
            event::trigger(WindowDestroyStartEvent{});
            result = 0;
            break;
        case WM_NCDESTROY:
            event::trigger(WindowDestroyEndEvent{});
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

/**
 * Creates a window
 * @param inst the current instance
 * @param title the title of the window
 * @param width the width of the client window
 * @param height the height of the client window
 * @return the created window, or NULL on failure
 */
HWND createWindow(HINSTANCE inst, char const *title, int width, int height) {
    // setup the window class
    WNDCLASSEXA window_class = {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = windowCallback,
        .hInstance = inst,
        .lpszClassName = WINDOW_CLASS_NAME,
    };

    // register the class
    if(!RegisterClassExA(&window_class)) {
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
        fprintf(stderr, "Failed to create window: %s\n", WINDOW_TITLE);

        // NULL on failure
        return 0;
    }

    return window;
}

/**
 * Setup an OpenGL context
 * @param dc the device context to render with
 * @param rd the OpenGL rendering context to render with
 * @param window the window to render onto
 * @return success or failure
 */
BOOL setupOpenGL(HDC *dc, HGLRC *rc, HWND window) {
    // get the device context for the current window
    *dc = GetDC(window);

    if(!(*dc)) {
        fprintf(stderr, "Failed to get DC\n");

        // false on failure
        return false;
    }

    // setup the pixel format
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

    int pixel_format = ChoosePixelFormat(*dc, &pfd);

    if(!pixel_format) {
        fprintf(stderr, "Failed to choose pixel format for the given DC\n");

        // false on failure
        return false;
    }

    if(!SetPixelFormat(*dc, pixel_format, &pfd)) {
        fprintf(stderr, "Failed to set pixel format for the given DC\n");

        // false on failure
        return false;
    }

    *rc = wglCreateContext(*dc);
    if(!(*rc)) {
        fprintf(stderr, "Failed to create a rendering context\n");

        // false on failure
        return false;
    }

    if(!wglMakeCurrent(*dc, *rc)) {
        fprintf(stderr, "Failed to make the context current\n");

        // false on failure
        return false;
    }

    HMODULE opengl = LoadLibraryA("opengl32.dll");

    if(!opengl) {
        fprintf(stderr, "Failed to find opengl32.dll: %s\n", WINDOW_TITLE);

        // false on failure
        return false;
    }

    if(!gladLoadGLUserPtr((GLADuserptrloadfunc) GetProcAddress, opengl)) {
        fprintf(stderr, "Failed to initialize GLAD: %s\n", WINDOW_TITLE);

        // false on failure
        return false;
    }

    // true on success
    return true;
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
    if (!AttachConsole(ATTACH_PARENT_PROCESS) && !AllocConsole()) {
        // welp, windows sucks
        return 0;
    }

    // yay, we have a console!
    // setup the crt io
    setupCRTIO();

    // start up the engine thread
    std::thread engine(engineInit);

    // create a window
    HWND window = createWindow(inst, WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);

    if(!window) {
        fprintf(stderr, "Could not create window %s, aborting\n", WINDOW_TITLE);

        // Windows wants 0 returned if message loop is not reached
        return 0;
    }

    // context variables (device and rendering)
    HDC dc;
    HGLRC rc;

    // opengl setup
    if(!setupOpenGL(&dc, &rc, window)) {
        fprintf(stderr, "Could not initialize OpenGL, aborting\n");

        // Windows wants 0 returned if message loop is not reached
        return 0;
    }

    /** Code won't work yet, OpenGL needs some more setup
    // TODO this is a shader program test that should be removed later
    ShaderProgram program = compileShaderProgram("shaders/basic_vert.glsl",
            "shaders/basic_frag.glsl");

    if(!program) {
        fprintf(stderr, "Failed to create shader program\n");

        return 0;
    }

    glUseProgram(program);

    // we just making a little triangle :)
    float vertices[] = {
        // position, color
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
    };

    // very basic VAO and VBO implementation
    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
            GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
            (void*) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
            (void*) (3 * sizeof(float)));
            */

    // set the viewport to the client window size
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // set the clear color for the context
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // necessary loop variables
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

        // clear the buffer
		glClear(GL_COLOR_BUFFER_BIT);

        // draw a triangle
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // swap buffers
        SwapBuffers(dc);
    }

    // destroy everything necessary
    wglMakeCurrent(0, 0);
    wglDeleteContext(rc);

    // Windows wants the wParam of the WM_QUIT message returned, we can choose
    // to disregard this
    return msg.wParam;
}
