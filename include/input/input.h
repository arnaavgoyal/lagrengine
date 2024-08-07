#ifndef INPUT_INPUT_H
#define INPUT_INPUT_H

#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <string>
#include <unordered_map>
#include <windows.h>

#include "engine.h"
#include "utils/event.h"

#define INPUT_MOD_SHIFT 0b001
#define INPUT_MOD_CTRL 0b010
#define INPUT_MOD_ALT 0b100

#define CHECK_KEY(states, vk) ((states[vk] >> 7) == 1)
#define CHECK_MOD(states, mods, mod_mask, vk) \
    ((!(mods & mod_mask)) ^ CHECK_KEY(states, vk))
#define CHECK_ALL_MODS(states, mods) \
    CHECK_MOD(states, mods, INPUT_MOD_SHIFT, VK_SHIFT)     \
    && CHECK_MOD(states, mods, INPUT_MOD_CTRL, VK_CONTROL) \
    && CHECK_MOD(states, mods, INPUT_MOD_ALT, VK_MENU)

enum class InputDeviceKind : unsigned char {
    mouse,
    keyboard
};

enum class InputActionKind : unsigned char {
    onkeydown,
    keypressed,
    onkeyup
};

struct InputBindInfo {
    InputDeviceKind device;
    InputActionKind action;
    unsigned char key;
    unsigned char modifiers;

    friend bool operator==(InputBindInfo const &x, InputBindInfo const &y) {
        return x.device == y.device
            && x.action == y.action
            && x.key == y.key
            && x.modifiers == y.modifiers;
    }
};

template <>
struct std::hash<InputBindInfo> {
    std::size_t operator()(InputBindInfo const &key) const {
        return (std::size_t)(
            ((unsigned)key.device << 24)
            & ((unsigned)key.action << 16)
            & ((unsigned)key.key << 8)
            & (key.modifiers)
        );
    }
};

struct InputContext {
    std::string name;
    std::unordered_map<InputBindInfo, void (*)(void *)> bindings;

    void bind() { }

    template <typename... Args>
    void bind(InputBindInfo input, void (*handler)(void *), Args... args) {
        bindings[input] = handler;
        bind(args...);
    }
};

struct KeyInput {
    static bool s;
    static unsigned char keystates[2][256];
    static InputContext context;
    static std::mutex context_sync;
    //static std::condition_variable cond;

    static void use(InputContext c) {
        context_sync.lock();
        fprintf(stderr, "using\n");
        context = c;
        context_sync.unlock();
    }

    /**
     * This function MUST be called on the same thread as the Win32
     *    message queue to work.
    */
    static void update() {
        //fprintf(stderr, "updating key states\n");

        // update key states

        s = !s;
        if (!GetKeyboardState(&keystates[s][0])) {
            fprintf(stderr, "%s -- failed to get keyboard state\n", __func__);
        }

        // call appropriate handlers based on bindings

        context_sync.lock();

        unsigned char *const keystate = keystates[s];
        unsigned char *const pastkeystate = keystates[!s];

        //fputc('\n', stderr);
        for (auto [input, handler] : context.bindings) {

            bool is_pressed = CHECK_KEY(keystate, input.key) && CHECK_ALL_MODS(keystate, input.modifiers);
            bool was_pressed = CHECK_KEY(pastkeystate, input.key) && CHECK_ALL_MODS(pastkeystate, input.modifiers);

            bool trigger_bind = 
                (input.action == InputActionKind::keypressed && is_pressed && was_pressed)
                || (input.action == InputActionKind::onkeydown && is_pressed && !was_pressed)
                || (input.action == InputActionKind::onkeyup && !is_pressed && was_pressed);
            
            //fprintf(stderr, "key 0x%x: r%d i%d w%d t%d s%d\n", input.key, keystate[input.key], is_pressed, was_pressed, trigger_bind, CHECK_MOD(keystate, input.modifiers, INPUT_MOD_SHIFT, VK_SHIFT));

            if (trigger_bind) { handler(nullptr); }
        }

        context_sync.unlock();

        //fprintf(stderr, "done\n");
    }

    // static void init() {

    //     static bool called = false;
    //     if (called) {
    //         return;
    //     }

    //     std::function<void (EngineTickEvent)> handler = [](EngineTickEvent){ update(); };
    //     event::registerListener(std::move(handler));
    // }

};

#undef CHECK_ALL_MODS
#undef CHECK_MOD
#undef CHECK_KEY

#endif
