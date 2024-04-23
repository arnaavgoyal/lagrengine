#include <chrono>
#include <cstring>
#include <ctime>
#include <format>

#include "utils/log.h"

std::string logNow() {
    char buf[500];
    auto time = std::chrono::system_clock::to_time_t(std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()));
    tm localtime;
    localtime_s(&localtime, &time);
    std::sprintf(buf, "%d:%d:%d", localtime.tm_hour, localtime.tm_min, localtime.tm_sec);
    return std::string("[") + std::string(buf) + "] ";
}
