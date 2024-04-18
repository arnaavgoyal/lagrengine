#include <cassert>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <stdio.h>
#include <thread>
#include <windows.h>

#include <glad/gl.h>
#include <glad/wgl.h>

#include "engine.h"
#include "utils/event.h"
#include "utils/shader.h"

#define WINDOW_CLASS_NAME "window"
#define WINDOW_TITLE "Lagrengine"
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 400

/**
 * I hate everything.
 * @param mod the module to load from
 * @param proc_name the procedure to load
 * @return the loaded procedure
 */
FARPROC getOpenGLFunction(HMODULE mod, char const *proc_name) {
    FARPROC proc = GetProcAddress(mod, proc_name);

    if(proc)
        return proc;

    proc = wglGetProcAddress(proc_name);

    return proc;
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
            //PostQuitMessage(0);
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
 * Registers the window class. Notably this is different than window creation
 * now since we need to actually create two windows and destroy one of them to
 * initialize OpenGL properly. Windows is the best programming platform :)
 * @return success or failure
 */
bool registerWindowClass(HINSTANCE inst, char const *class_name) {
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
BOOL setupOpenGL(HDC *dc, HGLRC *rc, HINSTANCE inst) {
    // create a dummy window
    HWND dummy_window = createWindow(inst, WINDOW_CLASS_NAME, WINDOW_TITLE,
            WINDOW_WIDTH, WINDOW_HEIGHT, false);

    // get the device context for the current window
    HDC dummy_dc = GetDC(dummy_window);

    if(!dummy_dc) {
        fprintf(stderr, "Failed to get dummy DC\n");

        // false on failure
        return false;
    }

    // setup the dummy pixel format
    PIXELFORMATDESCRIPTOR dummy_pfd = {
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

    int dummy_pixel_format = ChoosePixelFormat(dummy_dc, &dummy_pfd);

    if(!dummy_pixel_format) {
        fprintf(stderr, "Failed to choose pixel format for the dummy DC\n");

        // false on failure
        return false;
    }

    if(!SetPixelFormat(dummy_dc, dummy_pixel_format, &dummy_pfd)) {
        fprintf(stderr, "Failed to set pixel format for the dummy DC\n");

        // false on failure
        return false;
    }

    // create a dummy rendering context
    HGLRC dummy_rc = wglCreateContext(dummy_dc);

    if(!dummy_rc) {
        fprintf(stderr, "Failed to create a dummy rendering context\n");

        // false on failure
        return false;
    }

    if(!wglMakeCurrent(dummy_dc, dummy_rc)) {
        fprintf(stderr, "Failed to make the dummy context current\n");

        // false on failure
        return false;
    }

    if(!gladLoadWGL(dummy_dc, (GLADloadfunc) wglGetProcAddress)) {
        fprintf(stderr, "Failed to load WGL functions\n");

        // false on failure
        return false;
    }

    wglMakeCurrent(0, 0);
    wglDeleteContext(dummy_rc);
    DestroyWindow(dummy_window);

    // create a window
    HWND window = createWindow(inst, WINDOW_CLASS_NAME, WINDOW_TITLE,
            WINDOW_WIDTH, WINDOW_HEIGHT, true);

    int pixel_format_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,
        0
    };

    *dc = GetDC(window);

    int pixel_format;
    unsigned int num_formats;
    wglChoosePixelFormatARB(*dc, pixel_format_attribs, 0, 1, &pixel_format,
            &num_formats);

    if(!num_formats) {
        fprintf(stderr, "Failed to choose the pixel format for the DC\n");

        // false on failure
        return false;
    }

    PIXELFORMATDESCRIPTOR pfd;
    DescribePixelFormat(*dc, pixel_format, sizeof(pfd), &pfd);

    if(!SetPixelFormat(*dc, pixel_format, &pfd)) {
        fprintf(stderr, "Failed to set pixel format for the DC\n");

        // false on failure
        return false;
    }

    int gl_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    *rc = wglCreateContextAttribsARB(*dc, 0, gl_attribs);

    if(!(*rc)) {
        fprintf(stderr, "Failed to create a rendering context\n");

        // false on failure
        return false;
    }

    wglMakeCurrent(*dc, *rc);

    HMODULE opengl = LoadLibraryA("opengl32.dll");

    if(!opengl) {
        fprintf(stderr, "Failed to load OpenGL32.dll\n");

        return false;
    }

    if(!gladLoadGLUserPtr((GLADuserptrloadfunc) getOpenGLFunction, opengl)) {
        fprintf(stderr, "Failed to initialize GLAD\n");

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

    // register the window
    if(!registerWindowClass(inst, WINDOW_CLASS_NAME)) {
        fprintf(stderr, "Could not register window class %s\n, aborting",
                WINDOW_CLASS_NAME);

        // return 0 since we did not reach message loop yet
        return 0;
    }

    // context variables (device and rendering)
    HDC dc;
    HGLRC rc;

    // opengl setup
    if(!setupOpenGL(&dc, &rc, inst)) {
        fprintf(stderr, "Could not initialize OpenGL, aborting\n");

        // Windows wants 0 returned if message loop is not reached
        return 0;
    }

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
