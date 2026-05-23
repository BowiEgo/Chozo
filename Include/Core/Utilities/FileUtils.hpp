#pragma once

#include <filesystem>

namespace CZ::FileUtils {

using Path = std::filesystem::path;

Path GetAbsolutePath(const Path& path);

bool CreateDirectoryIfNeeded(std::string directory);

Path GetProjectRoot();

const Path GetResourcesDirectory();

const Path GetShaderSourcesDirectory();

const Path GetAssetDirectory();

const Path GetShaderCacheDirectory();

const Path GetThumbnailCacheDirectory();

std::string ReadTextFile(const Path& filepath);

bool ReadBinaryFile(const std::string& filepath, std::vector<uint32_t>& target);

void DeleteFile(const std::string& filepath);

std::string BytesToHumanReadable(uint64_t bytes);

} // namespace CZ::FileUtils