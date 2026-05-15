#include <Core/Platform/Mac/MacFile.h>

#include <mach-o/dyld.h>

namespace CZ::Platform::File {

std::filesystem::path GetExecutablePath() {
    std::filesystem::path result;
    char buffer[1024];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0) {
        result = std::filesystem::path(buffer).lexically_normal();
    }
    else {
        std::string path(size, '\0');
        _NSGetExecutablePath(path.data(), &size);
        result = std::filesystem::path(path).lexically_normal();
    }

    std::filesystem::current_path(result.parent_path());
    
    return result;
}

}