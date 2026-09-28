#include <Core/Platform/Windows/WindowsFile.h>

// clang-format off
#include <windows.h>
// clang-format on

#include <string>
#include <vector>

namespace CZ::Platform::File {

std::filesystem::path GetExecutablePath() {
    // Query the required size first, then read the UTF-16 path and convert it.
    DWORD size = GetModuleFileNameW(nullptr, nullptr, 0);
    if (size == 0) return {};

    std::wstring buffer(size, L'\0');
    DWORD written = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (written == 0) return {};

    buffer.resize(written);

    std::filesystem::path result = std::filesystem::path(buffer).lexically_normal();
    std::error_code ec;
    std::filesystem::current_path(result.parent_path(), ec);

    return result;
}

} // namespace CZ::Platform::File
