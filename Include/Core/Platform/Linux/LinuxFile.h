#pragma once

#include <filesystem>

namespace CZ::Platform::File {

/// Absolute path of the running executable (read from /proc/self/exe).
/// Linux support is a placeholder for now: only the core modules are expected to build.
std::filesystem::path GetExecutablePath();

} // namespace CZ::Platform::File
