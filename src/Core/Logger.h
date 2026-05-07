#pragma once

#include <iostream>
#include <sstream>

namespace Debugger3DS::Logging {

/// @brief Lightweight stream-style logger.
///
/// Writes to the stream pointed to by #output (defaults to @c std::cout).
/// Set @c enabled = false to suppress all output (e.g. during bulk parsing).
/// Redirect #output to a custom @c std::ostream to capture output separately
/// from raw @c std::cout writes.
class Logger {
public:
    /// @brief Write any streamable value.
    template<typename T>
    Logger& operator<<(const T& value) {
        if (enabled) {
            *output << value;
        }
        return *this;
    }

    /// @brief Handle stream manipulators such as @c std::endl.
    Logger& operator<<(std::ostream& (*manip)(std::ostream&)) {
        if (enabled) {
            *output << manip;
        }
        return *this;
    }

    /// @brief When @c false all output is silently discarded.
    static inline bool enabled = true;
    /// @brief Target output stream. Defaults to @c std::cout.
    static inline std::ostream* output = &std::cout;
};

/// @brief Global logger instance.
inline Logger log;

} // namespace Debugger3DS::Logging

/// @brief Backward-compatible alias so existing `logging::` call sites continue to compile.
namespace logging = Debugger3DS::Logging;
