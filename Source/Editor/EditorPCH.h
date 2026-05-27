#pragma once

#include <Core/Log/LogMacros.hpp>
#include <Core/Memory/Memory.hpp>
#include <Core/Memory/MemoryTypes.hpp>

#include "imgui.h"

#include <vulkan/vulkan_core.h>
#define IM_VULKAN_HAS_DYNAMIC_RENDERING
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"

#include <atomic>
#include <cctype>  // Required for toupper()
#include <cstdarg> // Required for va_list, va_start, va_end
#include <cstdio>  // Required for vsnprintf
#include <cstring> // Required for memcpy, strlen, strstr
#include <filesystem>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
