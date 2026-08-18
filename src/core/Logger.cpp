#include "core/Logger.h"

#include <android/log.h>
#include <cstdarg>
#include <cstdio>

namespace {

constexpr const char* TAG = "LeviIntegrated";

void vlog(int priority, const char* fmt, va_list args) {
    char buffer[1024];

    std::vsnprintf(
        buffer,
        sizeof(buffer),
        fmt,
        args
    );

    __android_log_print(
        priority,
        TAG,
        "%s",
        buffer
    );
}

} // namespace

namespace levi::Logger {

void info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vlog(
        ANDROID_LOG_INFO,
        fmt,
        args
    );

    va_end(args);
}

void warn(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vlog(
        ANDROID_LOG_WARN,
        fmt,
        args
    );

    va_end(args);
}

void error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vlog(
        ANDROID_LOG_ERROR,
        fmt,
        args
    );

    va_end(args);
}

} // namespace levi::Logger
