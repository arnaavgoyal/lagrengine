#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <syncstream>
#include <thread>

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
#include "utils/event.h"
#include "utils/registry.h"

// ticks ------------------------------

static const unsigned TICKRATE = 64;

void tickTrigger() {
    static auto interval = std::chrono::seconds(1) / TICKRATE;
    while (true) {
        std::this_thread::sleep_for(interval);
        event::trigger(EngineTickEvent{});
    }
}

// movement ------------------------------

static float const cam_speed = 0.00002f;

static glm::vec3 cam_pos(0.0f, 0.0f, 5.0f);
static glm::vec3 cam_front(0.0f, 0.0f, -1.0f);
static glm::vec3 cam_up(0.0f, 1.0f, 0.0f);
static float cam_fov = 45.0f;

static float yaw = -90.0f;
static float pitch = 0.0f;
static float sens = 0.0002f;

void moveForward(void *) {
    cam_pos += cam_speed * cam_front;
}

void moveLeft(void *) {
    cam_pos -= glm::normalize(glm::cross(cam_front, cam_up)) * cam_speed;
}

void moveBackward(void *) {
    cam_pos -= cam_speed * cam_front;
}

void moveRight(void *) {
    cam_pos += glm::normalize(glm::cross(cam_front, cam_up)) * cam_speed;
}

void moveUp(void *) {
    cam_pos += cam_speed * cam_up;
}

void moveDown(void *) {
    cam_pos -= cam_speed * cam_up;
}

void lookUp(void *) {
    pitch += sens;
}

void lookDown(void *) {
    pitch -= sens;
}

void lookLeft(void *) {
    yaw -= sens;
}

void lookRight(void *) {
    yaw += sens;
}

// void lookAround(void *) {
//     static float yaw = -90.0f;
//     static float pitch = 0.0f;
//     static float sens = 0.1f;

//     if (!mouse_move_cam) { return; }

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
// }

// engine ------------------------------

static bool run = true;
void terminate(WindowCloseRequestedEvent) {
    run = false;
}

int engineInit(OpenGLWrapper graphics) {

    // start tick trigger thread
    std::thread tick_thread(tickTrigger);
    tick_thread.detach();

    // opengl setup
    graphics.initGL();

    ShaderProgram program;
    if(!program.create("assets/shaders/basic_vert.glsl",
                "assets/shaders/basic_frag.glsl")) {
        fprintf(stderr, "Failed to create shader program\n");
        return 1;
    }

    program.use();

    // set the clear color for the context
    glClearColor(0.0f, 0.2f, 0.8f, 1.0f);

    // enable depth
    glEnable(GL_DEPTH_TEST);

    Registry<std::string, Model> model_reg;

    Model elephant_model;
    auto path = "assets/elephant/Mesh_Elephant.obj";
    elephant_model.create(path);
    Handle elephant_handle = model_reg.put(path, elephant_model);

    Scene scene;
    Camera cam;
    cam.init(cam_pos, cam_front, cam_up, 45.0f,
            (float) graphics.width / (float) graphics.height);
    SceneObject &elephant_object1 = scene.addObject(
        model_reg[elephant_handle],
        glm::translate(
            glm::scale(
                glm::mat4(1.0f),
                glm::vec3(0.01f, 0.01f, 0.01f)
            ),
            glm::vec3(100, 0, 0)
        ),
        program
    );

    SceneObject &elephant_object2 = scene.addObject(
        model_reg[elephant_handle],
        glm::translate(
            glm::scale(
                glm::mat4(1.0f),
                glm::vec3(0.01f, 0.01f, 0.01f)
            ),
            glm::vec3(-100, 0, 0)
        ),
        program
    );

    // set keyboard input handlers
    //KeyInput::init();
    InputContext ic;
    ic.bind(
        InputBindInfo{ InputDeviceKind::keyboard, InputActionKind::keypressed, 0x57, 0 }, moveForward, // w
        InputBindInfo{ InputDeviceKind::keyboard, InputActionKind::keypressed, 0x41, 0 }, moveLeft, // a
        InputBindInfo{ InputDeviceKind::keyboard, InputActionKind::keypressed, 0x53, 0 }, moveBackward, // s
        InputBindInfo{ InputDeviceKind::keyboard, InputActionKind::keypressed, 0x44, 0 }, moveRight, // d
        InputBindInfo{ InputDeviceKind::keyboard, InputActionKind::keypressed, VK_SPACE, 0 }, moveUp,
        InputBindInfo{ InputDeviceKind::keyboard, InputActionKind::keypressed, VK_SPACE, INPUT_MOD_SHIFT }, moveDown,

        InputBindInfo{ InputDeviceKind::keyboard, InputActionKind::keypressed, VK_UP, 0 }, lookUp,
        InputBindInfo{ InputDeviceKind::keyboard, InputActionKind::keypressed, VK_LEFT, 0 }, lookLeft,
        InputBindInfo{ InputDeviceKind::keyboard, InputActionKind::keypressed, VK_DOWN, 0 }, lookDown,
        InputBindInfo{ InputDeviceKind::keyboard, InputActionKind::keypressed, VK_RIGHT, 0 }, lookRight
    );
    KeyInput::use(ic);

    auto init_time
        = std::chrono::system_clock::now().time_since_epoch()
        / std::chrono::milliseconds(10);

    while(run) {

        event::waitFor<EngineTickEvent>();

        auto angle_diff = 0.7f;
        auto last_angle = 1.0f;

        elephant_object1.world = glm::rotate(
            elephant_object1.world,
            last_angle * glm::radians(1.0f),
            glm::normalize(glm::vec3(1.0f, 0.5f, 0.0f))
        );
        last_angle += angle_diff;

        glm::vec3 dir(
            glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch)),
            glm::sin(glm::radians(pitch)),
            glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch))
        );
        cam_front = glm::normalize(dir);

        cam.pos = cam_pos;
        cam.front = cam_front;
        cam.up = cam_up;

        // clear the buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        scene.draw(cam);

        // swap buffers
        graphics.swapBuffers();
    }

    // clean everything up
    graphics.destroy();
    elephant_model.destroy(); // this is bad with registry but im lazy
    program.destroy();

    return 0;
}
