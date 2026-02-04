#pragma once

#include "LogMacros.h"
#include "Logger.h"

#include <fstream>
#include <iomanip>
#include <regex>
#include <sstream>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

namespace ChozoUtils::File {

DECLARE_LOG_CATEGORY_EXTERN(LogFileUtils, Info);

static const std::regex imagePattern(R"(\.(png|jpg|jpeg|hdr)$)",
                                     std::regex::icase);
static const std::regex hdrPattern(R"(\.(hdr)$)", std::regex::icase);
static const std::regex scenePattern(R"(\.(chozo)$)", std::regex::icase);

static const bool IsImage(std::string path) {
    std::filesystem::path filePath = std::filesystem::path(path);
    std::string fileExtension = filePath.extension().string();
    return std::regex_match(fileExtension, imagePattern);
}

std::filesystem::path GetExecutablePath();

static std::filesystem::path
    GetAbsolutePath(const std::filesystem::path &path) {
    std::filesystem::path result;

    if (path.is_relative()) {
        std::filesystem::path exePath =
            std::filesystem::absolute(ChozoUtils::File::GetExecutablePath())
                .parent_path();
        result = (exePath / path).lexically_normal();
    }

    result = std::filesystem::absolute(result).lexically_normal();

    std::wstring dirStr = result.wstring();
    std::replace(dirStr.begin(), dirStr.end(), L'\\', L'/');

    return std::filesystem::path(dirStr);
}

static bool CreateDirectoryIfNeeded(std::string directory) {
    if (!std::filesystem::exists(directory))
        return std::filesystem::create_directories(directory);

    return false;
}

static std::filesystem::path GetProjectRoot() {
    std::filesystem::path projectRoot;

    const char *envPath = std::getenv("CHOZO_ROOT");

    if (envPath) {
        projectRoot = std::filesystem::path(envPath);
    } else {
        projectRoot = std::filesystem::current_path();
        CZ_LOG(
            LogFileUtils, Warning,
            "CHOZO_ROOT environment variable not found! Falling back to: {0}",
            projectRoot.string());
    }

    return projectRoot;
}

static const std::filesystem::path GetResourcesDirectory() {
    return GetAbsolutePath(std::filesystem::path("../resources"));
}

static const std::filesystem::path GetShaderSourcesDirectory() {
    return GetAbsolutePath(
        std::filesystem::path("../../../../ChozoEngine/shaders"));
}

static const std::filesystem::path GetAssetDirectory() {
    // TODO: make sure the assets directory is valid
    auto path = GetAbsolutePath(std::filesystem::path("./assets"));
    CreateDirectoryIfNeeded(path.string());
    return path;
}

static const std::filesystem::path GetShaderCacheDirectory() {
    // TODO: make sure the assets directory is valid
    auto path = GetAbsolutePath(std::filesystem::path("./caches/shader"));
    CreateDirectoryIfNeeded(path.string());
    return path;
}

static const std::filesystem::path GetThumbnailCacheDirectory() {
    // TODO: make sure the assets directory is valid
    auto path = GetAbsolutePath(std::filesystem::path("./caches/thumbnail"));
    CreateDirectoryIfNeeded(path.string());
    return path;
}

static std::string ReadTextFile(const std::filesystem::path &filepath) {
    std::string result;
    std::ifstream in(filepath, std::ios::in | std::ios::binary | std::ios::ate);
    if (in) {
        std::streamsize size = in.tellg();
        if (size > 0) {
            result.resize(static_cast<size_t>(size));
            in.seekg(0, std::ios::beg);

            if (!in.read(result.data(), size)) {
                CZ_LOG(LogFileUtils, Error,
                       "Failed to read content from file: {0}",
                       filepath.string());
                result.clear();
            }
        } else {
            CZ_LOG(LogFileUtils, Warning, "File is empty: {0}",
                   filepath.string());
        }
        in.close();
    } else {
        CZ_LOG(LogFileUtils, Error, "Could not open file '{0}'",
               filepath.string());
    }

    return result;
}

static bool ReadBinaryFile(const std::string &filepath,
                           std::vector<uint32_t> &target) {
    std::ifstream in(filepath, std::ios::in | std::ios::binary);
    if (in) {
        in.seekg(0, std::ios::end);
        auto size = in.tellg();
        in.seekg(0, std::ios::beg);

        target.resize(size / sizeof(uint32_t));
        in.read((char *)target.data(), size);
        return true;
    } else {
        CZ_LOG(LogFileUtils, Error, "Could not open file '{0}'", filepath);
        return false;
    }
}

static void DeleteFile(const std::string &filepath) {
    try {
        if (std::filesystem::exists(filepath)) {
            std::filesystem::remove(filepath);
            CZ_LOG(LogFileUtils, Trace, "File at {} deleted successfully",
                   filepath);
        } else {
            CZ_LOG(LogFileUtils, Warning, "File at {} not found", filepath);
        }
    } catch (const std::filesystem::filesystem_error &err) {
        CZ_LOG(LogFileUtils, Error, "Error: {}  {}", err.what(), filepath);
    }
}

static std::string BytesToHumanReadable(uint64_t bytes) {
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