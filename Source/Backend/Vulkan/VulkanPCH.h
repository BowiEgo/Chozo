#pragma once

#include <Core/Header/Assert.hpp>
#include <Core/Header/Macros.h>
#include <Core/Header/Result.hpp>
#include <Core/Log/LogMacros.hpp>
#include <Core/Memory/Memory.hpp>
#include <Core/Platform/Platform.h>

#include <Runtime/RHI/RHITypes.hpp>

// ---------- Vulkan platform macros MUST come first ----------
#ifdef CZ_PLATFORM_MACOS
    #define VK_USE_PLATFORM_METAL_EXT
#endif

// Optional portability subset fallback
#ifndef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
    #define VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME "VK_KHR_portability_subset"
#endif

// Now VMA header, which internally includes vulkan.h, will see the macro
#include "vk_mem_alloc.h"
#include <vulkan/vulkan.h> // redundant but harmless due to include guard
#ifdef VK_USE_PLATFORM_METAL_EXT
    #include <vulkan/vulkan_metal.h>
#endif

#include "VulkanUtils.hpp"

#include <algorithm>
#include <cstring>
#include <optional>
#include <set>
#include <vector>
