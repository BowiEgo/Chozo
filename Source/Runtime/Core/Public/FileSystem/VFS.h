#pragma once

#include "LogMacros.h"
#include "Logger.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVFS, Info);

class CORE_API VFS {
public:
    static constexpr const char* ENGINE_PROTOCOL = "engine://";

    // Set the root directory for specific protocols (e.g., "shaders://")
    static void Mount(const std::string& protocol, const std::filesystem::path& physicalPath);

    // The "Resolve" function that converts virtual paths to absolute disk paths
    static std::filesystem::path Resolve(const std::string& virtualPath);

private:
    // Internal storage for path mappings
    static std::unordered_map<std::string, std::filesystem::path> s_PathProtocols;
};