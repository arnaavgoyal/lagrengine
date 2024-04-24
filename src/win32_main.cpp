#include <cassert>
#include <fcntl.h>
#include <fstream>
#include <io.h>
#include <iostream>
#include <stdio.h>
#include <thread>
#include <windows.h>

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
#include "graphics/shader.h"
#include "graphics/texture.h"
#include "graphics/vertex.h"
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

    // start up the engine thread
    std::thread engine(engineInit);

    // register the window
    if(!registerWindowClass(inst, WINDOW_CLASS_NAME, windowCallback)) {
        fprintf(stderr, "Could not register window class %s\n, aborting",
                WINDOW_CLASS_NAME);

        // return 0 since we did not reach message loop yet
        return 0;
    }

    // context variables (device and rendering)
    HDC dc;
    HGLRC rc;
    HWND window;

    // opengl setup
    OpenGLWrapper graphics;
    if(!graphics.init(&window, &dc, &rc, inst, WINDOW_CLASS_NAME,
        WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT)) {
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

    graphics.useShaderProgram(program);

    // we just making a BIG CUBE :D
    //
    //       5 - - - 4
    //    /  |    /  |
    // 1 - - - 0     |
    // |     6 | - - 7
    // |       |  /
    // 2 - - - 3
    //
    // 0 (+, +, +)
    // 1 (-, +, +)
    // 2 (-, -, +)
    // 3 (+, -, +)
    // 4 (+, +, -)
    // 5 (-, +, -)
    // 6 (-, -, -)
    // 7 (+, -, -)
    Vertex raw_vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    unsigned raw_indices[] = {
         0,  1,  2,  3,  4,  5,
         6,  7,  8,  9, 10, 11,
        12, 13, 14, 15, 16, 17,
        18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29,
        30, 31, 32, 33, 34, 35
    };

    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;
    vertices.assign(raw_vertices, raw_vertices + sizeof(raw_vertices) / sizeof(Vertex));
    indices.assign(raw_indices, raw_indices + sizeof(raw_indices) / sizeof(unsigned));

    // // we just making a little triangle :)
    // std::vector<Vertex> vertices;
    // std::vector<unsigned int> indices;
    // Vertex v0 = {
    //     {-0.5f, -0.5f, 0.0f},
    //     {0.0f, 0.0f},
    // };
    // Vertex v1 = {
    //     {0.5f, -0.5f, 0.0f},
    //     {1.0f, 0.0f},
    // };
    // Vertex v2 = {
    //     {-0.5f, 0.5f, 0.0f},
    //     {0.0f, 1.0f},
    // };
    // Vertex v3 = {
    //     {0.5f, 0.5f, 0.0f},
    //     {1.0f, 1.0f},
    // };
    // vertices.push_back(v0);
    // vertices.push_back(v1);
    // vertices.push_back(v2);
    // vertices.push_back(v3);
    // indices.push_back(0);
    // indices.push_back(1);
    // indices.push_back(2);
    // indices.push_back(1);
    // indices.push_back(2);
    // indices.push_back(3);


    Texture t;
    t.create("test.png");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE, t.id);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);

    // very basic VAO and VBO implementation
    //std::ifstream objfile("cat.obj");
    Mesh m;
    m.create(vertices, indices);
    //m.createFromObj(objfile);

    // set the viewport to the client window size
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // set the clear color for the context
    glClearColor(0.0f, 0.2f, 0.8f, 1.0f);

    // enable depth
    glEnable(GL_DEPTH_TEST);

    glUseProgram(program);

    glm::mat4 proj_tr_mat = glm::perspective(
        glm::radians(45.0f),
        (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,
        0.1f,
        100.0f
    );

    glUniformMatrix4fv(
        glGetUniformLocation(program, "proj"),
        1,
        GL_FALSE,
        glm::value_ptr(proj_tr_mat)
    );
  
    //graphics.initPipeline(sizeof(vertices), vertices);

    // necessary loop variables
    bool running = true;
    MSG msg;
    auto init_time
        = std::chrono::system_clock::now().time_since_epoch()
        / std::chrono::milliseconds(10);

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

        auto time_diff
            = std::chrono::system_clock::now().time_since_epoch()
            / std::chrono::milliseconds(10)
            - init_time;

        glm::mat4 model_tr_mat(1.0f);
        model_tr_mat = glm::rotate(
            model_tr_mat,
            (float)time_diff * glm::radians(1.0f),
            glm::vec3(1.0f, 0.5f, 0.0f)
        );

        glm::mat4 view_tr_mat(1.0f);
        view_tr_mat = glm::translate(
            view_tr_mat,
            glm::vec3(0.0f, 0.0f, -3.0f)
        );

        glUniformMatrix4fv(
            glGetUniformLocation(program, "model"),
            1,
            GL_FALSE,
            glm::value_ptr(model_tr_mat)
        );

        glUniformMatrix4fv(
            glGetUniformLocation(program, "view"),
            1,
            GL_FALSE,
            glm::value_ptr(view_tr_mat)
        );

        // clear the buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m.draw();

        // swap buffers
        SwapBuffers(dc);
      
        //graphics.doDrawIteration();
    }

    graphics.destroy();

    // Windows wants the wParam of the WM_QUIT message returned, we can choose
    // to disregard this
    return msg.wParam;
}
