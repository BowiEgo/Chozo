#include <Core/FileSystem/VFS.hpp>

#include <Core/Log/LogMacros.hpp>

namespace CZ {

std::unordered_map<std::string, std::filesystem::path> VFS::s_PathProtocols;

void VFS::Mount(const std::string& protocol, const std::filesystem::path& physicalPath) {

    // Ensure the protocol ends with "://" for consistency
    std::string key = protocol;
    if (key.find("://") == std::string::npos) {
        key += "://";
    }
    s_PathProtocols[key] = physicalPath;
}

std::filesystem::path VFS::Resolve(const std::string& virtualPath) {
    std::filesystem::path resolvedPath;
    bool found = false;

    for (const auto& [protocol, physicalRoot] : s_PathProtocols) {
        if (virtualPath.compare(0, protocol.length(), protocol) == 0) {
            // Extract the relative part after "protocol://"
            std::string relativePath = virtualPath.substr(protocol.length());

            size_t firstNotSlash = relativePath.find_first_not_of("/\\");
            if (firstNotSlash != std::string::npos) {
                relativePath = relativePath.substr(firstNotSlash);
            }

            resolvedPath = physicalRoot / relativePath;
            found        = true;
            break;
        }
    }

    if (!found) {
        resolvedPath = std::filesystem::path(virtualPath);
    }

    if (!std::filesystem::exists(resolvedPath)) {
        CZ_CORE_LOG(Warning, "File not found: ", resolvedPath.string(), virtualPath);
    }

    return resolvedPath;
}
} // namespace CZ