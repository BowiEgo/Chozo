#pragma once

#include <Core/Memory/MemoryTypes.hpp>
#include <Core/Memory/TypeMemoryTracker.hpp>

#include <filesystem>

namespace CZ {

struct VFS {
    static std::unordered_map<std::string, std::filesystem::path> s_PathProtocols;

    static constexpr const char* ENGINE_PROTOCOL = "engine://";

    // Set the root directory for specific protocols (e.g., "shaders://")
    static void Mount(const std::string& protocol, const std::filesystem::path& physicalPath);

    // The "Resolve" function that converts virtual paths to absolute disk paths
    static std::filesystem::path Resolve(const std::string& virtualPath);
};

} // namespace CZ