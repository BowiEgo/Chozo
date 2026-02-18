#pragma once

static std::string GetPlatformLibName(const std::string& name) {
#if defined(CZ_PLATFORM_WINDOWS)
    return name + ".dll";
#elif defined(CZ_PLATFORM_MACOS)
    return "lib" + name + ".dylib";
#else
    return "lib" + name + ".so";
#endif
}