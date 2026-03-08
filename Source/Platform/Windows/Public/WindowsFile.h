#pragma once

#include "PlatformTypes.h"

#include <filesystem>

namespace ChozoUtils::File {

std::filesystem::path GetExecutablePath();

int GetFileIconIndex(const std::filesystem::path& path);

FRawFileImage GetFileIcon(const std::filesystem::path& path);

FRawFileImage GetFileThumbnail(const std::filesystem::path& path, int size);

bool IsHiddenOrSystem(const std::filesystem::path& p);

} // namespace ChozoUtils::File