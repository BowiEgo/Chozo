#pragma once

#include <filesystem>

namespace CZ::Platform::File {

/// Absolute path of the running executable (the engine resolves its resource directories
/// relative to it). Also sets the current working directory to the executable's folder, matching
/// the macOS implementation.
std::filesystem::path GetExecutablePath();

} // namespace CZ::Platform::File
