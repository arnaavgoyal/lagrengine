#ifndef UTILS_LOG_H
#define UTILS_LOG_H

#include <chrono>
#include <mutex>
#include <ostream>
#include <string>
#include <thread>

#include "utils/tsq.h"

class Log {
private:

    struct LogEntry {
        std::chrono::time_point<std::chrono::system_clock> timestamp;
        std::string msg;
    };

    std::ostream &os;
    TSQ<LogEntry> queue;

    static void runImpl(std::ostream &os, TSQ<LogEntry> &queue) {
        while (true) {
            LogEntry le = queue.pop();
            os << "[" << std::chrono::floor<std::chrono::seconds>(le.timestamp) << "] " << le.msg << "\n";
        }
    }

public:
    
    Log(std::ostream &os) : os(os) {
        std::thread t(runImpl, std::ref(os), std::ref(queue));
        t.detach();
    }

    void log(std::string msg) {
        queue.push(LogEntry{std::chrono::system_clock::now(), msg});
    }

    Log &operator<<(std::string msg) {
        log(msg);
        return *this;
    }
};

#endif
