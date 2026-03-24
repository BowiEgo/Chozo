#pragma once

#include <cstdint>
#include <string>

#ifdef CZ_PLATFORM_WINDOWS
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <windows.h>
#elif defined(CZ_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11 // or WAYLAND
#elif defined(CZ_PLATFORM_MACOS)
    #define GLFW_EXPOSE_NATIVE_COCOA
#endif

struct FRawFileImage {
    std::string Name;
    std::string PathU8;
    uint8_t* Data = nullptr;
    int Size = 0;
    uint32_t Width = 0, Height = 0;
    int Index = -1;
    char Format = 0; // { 0 = BGRA, 1 = RGBA }
};