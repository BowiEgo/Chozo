#include <filesystem>

#include <cstddef>

#include <Core/Platform/Linux/LinuxFile.h>

#include <climits>
#include <string>
#include <unistd.h>

namespace CZ::Platform::File {

std::filesystem::path GetExecutablePath() {
    std::string buffer(PATH_MAX, '\0');
    const ssize_t length = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
    if (length <= 0) return {};

    buffer.resize(static_cast<size_t>(length));

    std::filesystem::path result = std::filesystem::path(buffer).lexically_normal();
    std::error_code ec;
    std::filesystem::current_path(result.parent_path(), ec);

    return result;
}

} // namespace CZ::Platform::File
