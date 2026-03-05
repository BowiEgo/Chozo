#include "FileUtils.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <regex>
#include <sstream>

namespace ChozoUtils::File {

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

namespace {

const std::regex s_ImagePattern(R"(\.(png|jpg|jpeg|hdr)$)", std::regex::icase);
const std::regex s_HdrPattern(R"(\.(hdr)$)", std::regex::icase);
const std::regex s_ScenePattern(R"(\.(chozo)$)", std::regex::icase);

} // namespace

std::filesystem::path GetExecutablePath() { return GetExecutablePath(); }

const bool IsImage(std::string path) {
    std::filesystem::path filePath = std::filesystem::path(path);
    std::string fileExtension = filePath.extension().string();
    return std::regex_match(fileExtension, s_ImagePattern);
}

std::filesystem::path GetAbsolutePath(const std::filesystem::path& path) {
    std::filesystem::path result;

    if (path.is_relative()) {
        std::filesystem::path exePath =
            std::filesystem::absolute(GetExecutablePath()).parent_path();
        result = (exePath / path).lexically_normal();
    }

    result = std::filesystem::absolute(result).lexically_normal();

    std::wstring dirStr = result.wstring();
    std::replace(dirStr.begin(), dirStr.end(), L'\\', L'/');

    return std::filesystem::path(dirStr);
}

bool CreateDirectoryIfNeeded(std::string directory) {
    if (!std::filesystem::exists(directory)) return std::filesystem::create_directories(directory);

    return false;
}

std::filesystem::path GetProjectRoot() {

    const char* envPath = std::getenv("CZ_ROOT");
    if (envPath) {
        std::string pathStr(envPath);
        // Clean up potential trailing semicolons from xmake/env path
        size_t last = pathStr.find_last_not_of(";");
        if (last != std::string::npos) {
            pathStr = pathStr.substr(0, last + 1);
        }
        return std::filesystem::path(pathStr);
    }

    try {
        std::filesystem::path current = std::filesystem::current_path();

        // Walk up from build/../debug/ to find the root
        while (current.has_parent_path()) {
            if (std::filesystem::exists(current / "Config") &&
                std::filesystem::exists(current / "Resources")) {
                return current;
            }
            current = current.parent_path();
        }
    } catch (const std::exception& e) {
        CZ_LOG(LogFileUtils, Error, "Error while determining project root: {}", e.what());
    }

    return std::filesystem::current_path();
}

const std::filesystem::path GetResourcesDirectory() {
    return GetAbsolutePath(std::filesystem::path("../resources"));
}

const std::filesystem::path GetShaderSourcesDirectory() {
    return GetAbsolutePath(std::filesystem::path("../../../../ChozoEngine/shaders"));
}

const std::filesystem::path GetAssetDirectory() {
    // TODO: make sure the assets directory is valid
    auto path = GetAbsolutePath(std::filesystem::path("./assets"));
    CreateDirectoryIfNeeded(path.string());
    return path;
}

const std::filesystem::path GetShaderCacheDirectory() {
    // TODO: make sure the assets directory is valid
    auto path = GetAbsolutePath(std::filesystem::path("./caches/shader"));
    CreateDirectoryIfNeeded(path.string());
    return path;
}

const std::filesystem::path GetThumbnailCacheDirectory() {
    // TODO: make sure the assets directory is valid
    auto path = GetAbsolutePath(std::filesystem::path("./caches/thumbnail"));
    CreateDirectoryIfNeeded(path.string());
    return path;
}

std::string ReadTextFile(const std::filesystem::path& filepath) {
    std::string result;
    std::ifstream in(filepath, std::ios::in | std::ios::binary | std::ios::ate);
    if (in) {
        std::streamsize size = in.tellg();
        if (size > 0) {
            result.resize(static_cast<size_t>(size));
            in.seekg(0, std::ios::beg);

            if (!in.read(result.data(), size)) {
                CZ_LOG(LogFileUtils, Error, "Failed to read content from file: {0}",
                       filepath.string());
                result.clear();
            }
        } else {
            CZ_LOG(LogFileUtils, Warning, "File is empty: {0}", filepath.string());
        }
        in.close();
    } else {
        CZ_LOG(LogFileUtils, Error, "Could not open file '{0}'", filepath.string());
    }

    return result;
}

bool ReadBinaryFile(const std::string& filepath, std::vector<uint32_t>& target) {
    std::ifstream in(filepath, std::ios::in | std::ios::binary);
    if (in) {
        in.seekg(0, std::ios::end);
        auto size = in.tellg();
        in.seekg(0, std::ios::beg);

        target.resize(size / sizeof(uint32_t));
        in.read((char*)target.data(), size);
        return true;
    } else {
        CZ_LOG(LogFileUtils, Error, "Could not open file '{0}'", filepath);
        return false;
    }
}

void DeleteFile(const std::string& filepath) {
    try {
        if (std::filesystem::exists(filepath)) {
            std::filesystem::remove(filepath);
            CZ_LOG(LogFileUtils, Trace, "File at {} deleted successfully", filepath);
        } else {
            CZ_LOG(LogFileUtils, Warning, "File at {} not found", filepath);
        }
    } catch (const std::filesystem::filesystem_error& err) {
        CZ_LOG(LogFileUtils, Error, "Error: {}  {}", err.what(), filepath);
    }
}

std::string BytesToHumanReadable(uint64_t bytes) {
    const uint64_t KB = 1024;
    const uint64_t MB = KB * 1024;
    const uint64_t GB = MB * 1024;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);

    if (bytes >= GB) {
        oss << static_cast<double>(bytes) / GB << " GB";
    } else if (bytes >= MB) {
        oss << static_cast<double>(bytes) / MB << " MB";
    } else if (bytes >= KB) {
        oss << static_cast<double>(bytes) / KB << " KB";
    } else {
        oss << bytes << " Bytes";
    }
    return oss.str();
}

} // namespace ChozoUtils::File