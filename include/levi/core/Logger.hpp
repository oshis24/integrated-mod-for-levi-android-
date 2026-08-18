#pragma once

#include <cstdarg>

namespace levi::core {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

class Logger final {
public:
    static void initialize();

    static void debug(const char* format, ...);
    static void info(const char* format, ...);
    static void warning(const char* format, ...);
    static void error(const char* format, ...);

private:
    static void write(
        LogLevel level,
        const char* format,
        va_list args
    );
};

} // namespace levi::core
