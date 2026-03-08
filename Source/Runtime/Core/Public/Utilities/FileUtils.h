#pragma once

#include "PlatformFile.h"

#include "LogMacros.h"
#include "Logger.h"

namespace ChozoUtils::File {

DECLARE_LOG_CATEGORY_EXTERN(LogFileUtils, Info);

CORE_API std::filesystem::path GetAbsolutePath(const std::filesystem::path& path);

CORE_API bool CreateDirectoryIfNeeded(std::string directory);

CORE_API std::filesystem::path GetProjectRoot();

CORE_API const std::filesystem::path GetResourcesDirectory();

CORE_API const std::filesystem::path GetShaderSourcesDirectory();

CORE_API const std::filesystem::path GetAssetDirectory();

CORE_API const std::filesystem::path GetShaderCacheDirectory();

CORE_API const std::filesystem::path GetThumbnailCacheDirectory();

CORE_API std::string ReadTextFile(const std::filesystem::path& filepath);

CORE_API bool ReadBinaryFile(const std::string& filepath, std::vector<uint32_t>& target);

CORE_API void DeleteFile(const std::string& filepath);
std::string BytesToHumanReadable(uint64_t bytes);

} // namespace ChozoUtils::File