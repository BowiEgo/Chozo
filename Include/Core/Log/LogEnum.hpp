#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace CZ {

using uint8 = std::uint8_t;

enum class LogVerbosity : uint8 {
    Fatal,   // Critical error, app cannot continue
    Error,   // Runtime error that requires attention
    Warning, // Unusual behavior that isn't an error yet
    Info,    // Major milestones or state changes
    Debug,   // Internal logic flow and variable states
    Trace    // High-frequency diagnostic or step-by-step tracing
};

using LogCallback = std::function<void(const std::string&, LogVerbosity)>;

} // namespace CZ