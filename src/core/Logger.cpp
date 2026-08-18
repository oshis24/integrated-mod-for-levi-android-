#include "levi/core/Logger.hpp"

#include <android/log.h>

#include <cstdarg>
#include <cstdio>

namespace levi::core {

namespace {

constexpr const char* kTag =
    "LeviModules";

void write(
    android_LogPriority priority,
    const char* fmt,
    va_list args
) noexcept {
    if (fmt == nullptr) {
        return;
    }

    char buffer[2048]{};

    std::vsnprintf(
        buffer,
        sizeof(buffer),
        fmt,
        args
    );

    __android_log_write(
        priority,
        kTag,
        buffer
    );
}

} // namespace

void Logger::initialize() noexcept {
    __android_log_write(
        ANDROID_LOG_INFO,
        kTag,
        "Logger initialized"
    );
}

void Logger::debug(
    const char* fmt,
    ...
) noexcept {
    va_list args;
    va_start(args, fmt);

    write(
        ANDROID_LOG_DEBUG,
        fmt,
        args
    );

    va_end(args);
}

void Logger::info(
    const char* fmt,
    ...
) noexcept {
    va_list args;
    va_start(args, fmt);

    write(
        ANDROID_LOG_INFO,
        fmt,
        args
    );

    va_end(args);
}

void Logger::warning(
    const char* fmt,
    ...
) noexcept {
    va_list args;
    va_start(args, fmt);

    write(
        ANDROID_LOG_WARN,
        fmt,
        args
    );

    va_end(args);
}

void Logger::error(
    const char* fmt,
    ...
) noexcept {
    va_list args;
    va_start(args, fmt);

    write(
        ANDROID_LOG_ERROR,
        fmt,
        args
    );

    va_end(args);
}

} // namespace levi::core
