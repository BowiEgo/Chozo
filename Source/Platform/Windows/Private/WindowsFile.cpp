#include "WindowsFile.h"

#include <windows.h>

namespace ChozoUtils::File {

std::filesystem::path GetExecutablePath() {
    wchar_t buffer[MAX_PATH] = { 0 };
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(buffer).lexically_normal();
}
} // namespace ChozoUtils::File