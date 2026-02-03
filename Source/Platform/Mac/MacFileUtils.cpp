#include <mach-o/dyld.h>

namespace Chozo {

namespace Utils::File {

fs::path GetExecutablePath() {
    char buffer[1024];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0)
        return fs::path(buffer).lexically_normal();
    else {
        std::string path(size, '\0');
        _NSGetExecutablePath(path.data(), &size);
        return fs::path(path).lexically_normal();
    }
}
} // namespace Utils::File
} // namespace Chozo