#pragma once

#include "PlatformTypes.h"

#include <filesystem>

namespace ChozoUtils::File {

std::filesystem::path GetExecutablePath();

FRawIcon GetIcon(const std::filesystem::path& path);

bool IsHiddenOrSystem(const std::filesystem::path& p);

} // namespace ChozoUtils::File