#include <Windows.h>

#include <filesystem>

namespace ChozoUtils {

namespace File {

inline std::filesystem::path GetExecutablePath() {
    wchar_t buffer[MAX_PATH] = {0};
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(buffer).lexically_normal();
}
} // namespace File
} // namespace ChozoUtils