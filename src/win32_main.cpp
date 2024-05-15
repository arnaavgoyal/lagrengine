#include <cassert>
#include <fcntl.h>
#include <fstream>
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
#include "os/window.h"
#include "utils/event.h"

#define WINDOW_CLASS_NAME "window"
#define WINDOW_TITLE "Lagrengine"
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 400

static float const cam_speed = 0.05f;

static glm::vec3 cam_pos(0.0f, 0.0f, 5.0f);
static glm::vec3 cam_front(0.0f, 0.0f, -1.0f);
static glm::vec3 cam_up(0.0f, 1.0f, 0.0f);
static float cam_fov = 45.0f;

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
    static bool mouse_move_cam = false;
    static int c_mouse_x = WINDOW_WIDTH / 2;
    static int c_mouse_y = WINDOW_HEIGHT / 2;
    LRESULT result;
    switch(msg) {
        case WM_CHAR: {
            switch(wParam) {
            case 'w':
                cam_pos += cam_speed * cam_front;
                break;
            case 'a':
                cam_pos -= glm::normalize(glm::cross(cam_front, cam_up)) * cam_speed;
                break;
            case 's':
                cam_pos -= cam_speed * cam_front;
                break;
            case 'd':
                cam_pos += glm::normalize(glm::cross(cam_front, cam_up)) * cam_speed;
                break;
            case ' ':
                cam_pos += cam_speed * cam_up;
                break;
            default:
                break;
            }
            result = 0;
            break;
        }
        case WM_RBUTTONDOWN:
            mouse_move_cam = true;
            SetCapture(window);
            c_mouse_x = GET_X_LPARAM(lParam);
            c_mouse_y = GET_Y_LPARAM(lParam);
            result = 0;
            break;
        case WM_RBUTTONUP:
            mouse_move_cam = false;
            ReleaseCapture();
            result = 0;
            break;
        case WM_MOUSEMOVE: {
            static float yaw = -90.0f;
            static float pitch = 0.0f;
            static float roll = 0.0f;
            static float sens = 0.1f;

            if (!mouse_move_cam) {
                result = 0;
                break;
            }

            int mouse_x = GET_X_LPARAM(lParam);
            int mouse_y = GET_Y_LPARAM(lParam);

            float off_x = mouse_x - c_mouse_x;
            float off_y = c_mouse_y - mouse_y;

            c_mouse_x = mouse_x;
            c_mouse_y = mouse_y;

            off_x *= sens;
            off_y *= sens;

            yaw += off_x;
            pitch += off_y;

            glm::vec3 dir(
                glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch)),
                glm::sin(glm::radians(pitch)),
                glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch))
            );
            cam_front = glm::normalize(dir);

            result = 0;
            break;
        }
        case WM_MOUSEWHEEL: {
            short delta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
            
            // towards user / downwards
            cam_fov -= (float)delta;

            if (cam_fov < 1.0f) {
                cam_fov = 1.0f;
            }
            else if (cam_fov > 89.0f) {
                cam_fov = 89.0f;
            }

            result = 0;
            break;
        }
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
    ShaderProgram program;
    if(!program.create("shaders/basic_vert.glsl", "shaders/basic_frag.glsl")) {
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
    std::vector<Texture> textures;
    textures.push_back(t);

    //std::ifstream objfile("cat.obj");
    Mesh m;
    m.create(vertices, indices, textures);
    //m.createFromObj(objfile);

    // set the clear color for the context
    glClearColor(0.0f, 0.2f, 0.8f, 1.0f);

    // enable depth
    glEnable(GL_DEPTH_TEST);

    Scene scene;
    Camera cam;
    cam.create(WINDOW_WIDTH, WINDOW_HEIGHT, cam_pos, cam_front, cam_up, 45.0f);
    SceneObject &cube = scene.addObject(m, glm::mat4(1.0f), program);

    Model m;
    m.create("elephant/Mesh_Elephant.obj");

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
        model_tr_mat = glm::scale(model_tr_mat, glm::vec3(0.01f, 0.01f, 0.01f));
        model_tr_mat = glm::rotate(
            model_tr_mat,

        cube.world = glm::rotate(
            glm::mat4(1.0f),
            (float)time_diff * glm::radians(1.0f),
            glm::vec3(1.0f, 0.5f, 0.0f)
        );
   
        glm::mat4 view_tr_mat(1.0f);
        view_tr_mat = glm::translate(
            view_tr_mat,
            glm::vec3(0.0f, 0.0f, -3.0f)
        );

        glUniformMatrix4fv(
            glGetUniformLocation(program.id, "model"),
            1,
            GL_FALSE,
            glm::value_ptr(model_tr_mat)
        );

        glUniformMatrix4fv(
            glGetUniformLocation(program.id, "view"),
            1,
            GL_FALSE,
            glm::value_ptr(view_tr_mat)
        );

        cam.pos = cam_pos;
        cam.front = cam_front;
        cam.up = cam_up;
        cam.fov = cam_fov;

        // clear the buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        graphics.drawModel(program, m);
        scene.draw(cam);

        // swap buffers
        SwapBuffers(dc);
      
        //graphics.doDrawIteration();
    }

    // clean everything up
    graphics.destroy();
    m.destroy();
    program.destroy();

    // Windows wants the wParam of the WM_QUIT message returned, we can choose
    // to disregard this
    return msg.wParam;
}
