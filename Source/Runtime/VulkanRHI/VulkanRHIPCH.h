#pragma once

#ifdef CZ_PLATFORM_WINDOWS
    #define VK_USE_PLATFORM_WIN32_KHR
#elif defined(CZ_PLATFORM_MACOS)
    #define VK_USE_PLATFORM_METAL_EXT
#elif defined(CZ_PLATFORM_LINUX)
    #define VK_USE_PLATFORM_XCB_KHR
#endif

#include <vulkan/vulkan_raii.hpp>

#include <memory>
#include <set>