#include <cassert>
#include <fcntl.h>
#include <fstream>
//#include <hidusage.h>
#include <io.h>
#include <iostream>
#include <stdio.h>
#include <thread>
#include <windows.h>
#include <windowsx.h>

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#undef GLM_ENABLE_EXPERIMENTAL
#include <glm/mat4x4.hpp>
#include <glad/wgl.h>

#include "engine.h"
#include "graphics/graphics.h"
#include "graphics/mesh.h"
#include "graphics/model.h"
#include "graphics/scene.h"
#include "graphics/shader.h"
#include "graphics/texture.h"
#include "graphics/vertex.h"
#include "input/input.h"
#include "os/window.h"
#include "utils/event.h"

#define WINDOW_CLASS_NAME "window"
#define WINDOW_TITLE "Lagrengine"
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 400

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
        // case WM_MOUSEMOVE: {
        //     static float yaw = -90.0f;
        //     static float pitch = 0.0f;
        //     static float sens = 0.1f;

        //     if (!mouse_move_cam) {
        //         result = 0;
        //         break;
        //     }

        //     int mouse_x = GET_X_LPARAM(lParam);
        //     int mouse_y = GET_Y_LPARAM(lParam);

        //     float off_x = mouse_x - c_mouse_x;
        //     float off_y = c_mouse_y - mouse_y;

        //     c_mouse_x = mouse_x;
        //     c_mouse_y = mouse_y;

        //     off_x *= sens;
        //     off_y *= sens;

        //     yaw += off_x;
        //     pitch += off_y;

        //     glm::vec3 dir(
        //         glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch)),
        //         glm::sin(glm::radians(pitch)),
        //         glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch))
        //     );
        //     cam_front = glm::normalize(dir);

        //     result = 0;
        //     break;
        // }
        // case WM_MOUSEWHEEL: {
        //     short delta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
            
        //     // towards user / downwards
        //     cam_fov -= (float)delta;

        //     if (cam_fov < 1.0f) {
        //         cam_fov = 1.0f;
        //     }
        //     else if (cam_fov > 89.0f) {
        //         cam_fov = 89.0f;
        //     }

        //     result = 0;
        //     break;
        // }
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

    // register for raw input
    // unsigned const NRID = 2;
    // RAWINPUTDEVICE rid[NRID];
    // rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    // rid[0].usUsage     = HID_USAGE_GENERIC_MOUSE;
    // rid[0].dwFlags     = 0;
    // rid[0].hwndTarget  = 0;
    // rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
    // rid[1].usUsage     = HID_USAGE_GENERIC_KEYBOARD;
    // rid[1].dwFlags     = 0;
    // rid[1].hwndTarget  = 0;
    // if (!RegisterRawInputDevices(rid, NRID, sizeof(RAWINPUTDEVICE))) {
    //     // error with raw input
    //     fprintf(stderr, "Could not register raw input\n");
    //     return 0;
    // }

    // register the window
    if(!registerWindowClass(inst, WINDOW_CLASS_NAME, windowCallback)) {
        fprintf(stderr, "Could not register window class %s\n, aborting",
                WINDOW_CLASS_NAME);

        // return 0 since we did not reach message loop yet
        return 0;
    }

    // set up the window
    OpenGLWrapper graphics;
    if(!graphics.init(inst, WINDOW_CLASS_NAME, WINDOW_TITLE, WINDOW_WIDTH,
                WINDOW_HEIGHT)) {
        fprintf(stderr, "Could not initialize the window, aborting\n");
        return 1;
    }

    // start up the engine thread
    std::thread engine_thread(engineInit, graphics);
    engine_thread.detach();

    MSG msg;
    bool running = true;
    while (running) {

        event::waitFor<EngineTickEvent>();

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

        KeyInput::update();
    }

    // Windows wants the wParam of the WM_QUIT message returned, we can choose
    // to disregard this
    return msg.wParam;
}
