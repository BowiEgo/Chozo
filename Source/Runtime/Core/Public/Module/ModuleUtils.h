#pragma once

using PFN_CreateDevice = IRHIDevice*(const FRHIDeviceCreateInfo&);

static std::string GetPlatformLibName(const std::string& name) {
#if defined(CHOZO_PLATFORM_WINDOWS)
    return name + ".dll";
#elif defined(CHOZO_PLATFORM_MACOS)
    return "lib" + name + ".dylib";
#else
    return "lib" + name + ".so";
#endif
}