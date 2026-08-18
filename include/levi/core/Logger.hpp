#pragma once

namespace levi::core {

class Logger final {
public:
    static void initialize() noexcept;

    static void debug(
        const char* fmt,
        ...
    ) noexcept;

    static void info(
        const char* fmt,
        ...
    ) noexcept;

    static void warning(
        const char* fmt,
        ...
    ) noexcept;

    static void error(
        const char* fmt,
        ...
    ) noexcept;
};

} // namespace levi::core
