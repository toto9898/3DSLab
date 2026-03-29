#pragma once

#include <iostream>
#include <sstream>

namespace logging {

class Logger {
public:
    // Template function to handle any type that std::cout can handle
    template<typename T>
    Logger& operator<<(const T& value) {
        if (enabled) {
            std::cout << value;
        }
        return *this;
    }

    // Handle stream manipulators like std::endl, std::flush, etc.
    Logger& operator<<(std::ostream& (*manip)(std::ostream&)) {
        if (enabled) {
            std::cout << manip;
        }
        return *this;
    }

    static inline bool enabled = true;
};

// Global instance
inline Logger log;

} // namespace logging
