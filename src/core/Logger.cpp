#include "levi/core/Logger.hpp"

#include <android/log.h>
#include <cstdio>

namespace levi::core {

namespace {

constexpr const char* kTag = "LeviModules";

const char* levelName(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warning:
            return "WARN";
        case LogLevel::Error:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

android_LogPriority androidPriority(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Debug:
            return ANDROID_LOG_DEBUG;
        case LogLevel::Info:
            return ANDROID_LOG_INFO;
        case LogLevel::Warning:
            return ANDROID_LOG_WARN;
        case LogLevel::Error:
            return ANDROID_LOG_ERROR;
        default:
            return ANDROID_LOG_INFO;
    }
}

} // namespace

void Logger::initialize() {
    __android_log_print(
        ANDROID_LOG_INFO,
        kTag,
        "[INFO] Logger initialized"
    );
}

void Logger::debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    write(LogLevel::Debug, format, args);
    va_end(args);
}

void Logger::info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    write(LogLevel::Info, format, args);
    va_end(args);
}

void Logger::warning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    write(LogLevel::Warning, format, args);
    va_end(args);
}

void Logger::error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    write(LogLevel::Error, format, args);
    va_end(args);
}

void Logger::write(
    LogLevel level,
    const char* format,
    va_list args
) {
    char message[2048];

    vsnprintf(
        message,
        sizeof(message),
        format,
        args
    );

    __android_log_print(
        androidPriority(level),
        kTag,
        "[%s] %s",
        levelName(level),
        message
    );
}

} // namespace levi::core
