#ifndef UTILS_THREAD_H
#define UTILS_THREAD_H

#include <functional>
#include <concepts>
#include <thread>
#include <type_traits>
#include <cassert>

#include "utils/tsq.h"

/**
 * A pool of threads that can be used to run any function
 * with return type void.
 * Functions can be queued to be run by a thread at any
 * point after construction. Once the thread pool is
 * initialized, tasks will be run in the order they were
 * added.
 */
class ThreadPool {
private:

    struct Response {
        unsigned code;
    };

    struct Command {
        enum { run, die } cmd;
        std::function<void()> func;
    };

    bool alive;
    unsigned num;
    TSQ<Command> commandQueue;
    TSQ<Response> responseQueue;

    static void runner(TSQ<Command> &commandQueue, TSQ<Response> &responseQueue) {
        Command cmd;
        while(true) {
            cmd = commandQueue.pop();
            switch (cmd.cmd) {
            case Command::die:
                return;
            default:
                break;
            }
            std::invoke(cmd.func);
            //responseQueue.push(Response{0});
        }
    }

public:

    template <typename Callable, typename... Args>
    requires std::invocable<Callable, Args...>
    ThreadPool &run(Callable func, Args &&... args) {
        Command cmd{
            Command::run,
            std::function<void()>(std::bind(func, std::forward<Args>(args)...))
        };
        commandQueue.push(cmd);
        return *this;
    }

    ThreadPool &add(unsigned n) {
        for (unsigned i = 0; i < n; i++) {
            std::thread t(runner, std::ref(commandQueue), std::ref(responseQueue));
            t.detach();
        }
        num += n;
        return *this;
    }

    ThreadPool &kill(unsigned n) {
        assert(n <= num && "kill more threads than are in the pool?");
        for (unsigned i = 0; i < n; i++) {
            Command dieCmd{Command::die};
            commandQueue.push(dieCmd);
        }
        num -= n;
        return *this;
    }

    ThreadPool &killAll() { kill(num); return *this; }

    unsigned size() { return num; }

    ThreadPool(unsigned n) : num(0) { add(n); }

    ~ThreadPool() {
        killAll();
    }
};

#endif
