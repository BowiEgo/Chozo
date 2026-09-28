#include <Core/Platform/Windows/WindowsFile.h>

// clang-format off
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
// clang-format on

#include <string>
#include <vector>

namespace CZ::Platform::File {

std::filesystem::path GetExecutablePath() {
    // GetModuleFileNameW truncates instead of reporting the required size, so grow the buffer
    // until the returned length fits.
    std::wstring buffer(MAX_PATH, L'\0');
    DWORD written = 0;

    for (;;) {
        written = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (written == 0) return {};        // failure
        if (written < buffer.size()) break; // the whole path fits

        buffer.resize(buffer.size() * 2);
    }

    buffer.resize(written);

    std::filesystem::path result = std::filesystem::path(buffer).lexically_normal();
    std::error_code ec;
    std::filesystem::current_path(result.parent_path(), ec);

    return result;
}

} // namespace CZ::Platform::File
