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

    /**
     * Queues a function to be run by the next available
     * thread in the thread pool.
     * All args are perfectly forwarded and bound to the
     * function at the time of this call. Be careful when
     * passing references as arguments and then modifying
     * them afterwards.
     * @param func the function to run
     * @param args the args to run it with
     * @return this threadpool instance for call chaining
     */
    template <typename Callable, typename... Args>
    requires std::invocable<Callable, Args...>
    ThreadPool &run(Callable &&func, Args &&... args) {
        Command cmd{
            Command::run,
            std::function<void()>(std::bind(std::forward<Callable>(func), std::forward<Args>(args)...))
        };
        commandQueue.push(cmd);
        return *this;
    }

    /**
     * Adds threads to the pool. They will immediately be
     * available for running.
     * @param n the number of threads to add
     * @return this threadpool instance for call chaining
     */
    ThreadPool &add(unsigned n) {
        for (unsigned i = 0; i < n; i++) {
            std::thread t(runner, std::ref(commandQueue), std::ref(responseQueue));
            t.detach();
        }
        num += n;
        return *this;
    }

    /**
     * Kills threads and removes them from the pool.
     * Threads are killed via a kill command, meaning
     * that some (or all) threads may not be killed
     * immediately.
     * @param n the number of threads to kill
     * @return this threadpool instance for call chaining
     */
    ThreadPool &kill(unsigned n) {
        assert(n <= num && "kill more threads than are in the pool?");
        for (unsigned i = 0; i < n; i++) {
            Command dieCmd{Command::die};
            commandQueue.push(dieCmd);
        }
        num -= n;
        return *this;
    }

    /**
     * Kills all of the threads in the pool.
     * May not take immediate effect due to other
     * commands in queue or non-idling threads.
     * @return this threadpool instance for call chaining
     */
    ThreadPool &killAll() { kill(num); return *this; }

    unsigned size() { return num; }

    ThreadPool(unsigned n) : num(0) { add(n); }

    ~ThreadPool() { killAll(); }
};

#endif
