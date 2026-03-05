#include "MacFile.h"

#include <mach-o/dyld.h>

namespace ChozoUtils::File {

std::filesystem::path GetExecutablePath() {
    char buffer[1024];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0)
        return std::filesystem::path(buffer).lexically_normal();
    else {
        std::string path(size, '\0');
        _NSGetExecutablePath(path.data(), &size);
        return std::filesystem::path(path).lexically_normal();
    }
}
} // namespace ChozoUtils::File