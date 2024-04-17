#include <chrono>
#include <format>

#include "utils/log.h"

std::string logNow() {
    return std::string("[") + std::format("{:%Y-%m-%d %H:%M:%S}", std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now())) + "] ";
}
