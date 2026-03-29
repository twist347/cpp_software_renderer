#pragma once

#include <string_view>

namespace sr {
    enum class Error {
        FileNotFound,
        FileReadFailed,
        InvalidFormat,
        InvalidDimensions,
        WindowCreationFailed,
        OutOfMemory,
    };

    constexpr auto to_string(Error error) noexcept -> std::string_view {
        switch (error) {
            case Error::FileNotFound:
                return "file not found";
            case Error::FileReadFailed:
                return "file read failed";
            case Error::InvalidFormat:
                return "invalid format";
            case Error::InvalidDimensions:
                return "invalid dimensions";
            case Error::WindowCreationFailed:
                return "window creation failed";
            case Error::OutOfMemory:
                return "out of memory";
        }
        return "unknown error";
    }
}
