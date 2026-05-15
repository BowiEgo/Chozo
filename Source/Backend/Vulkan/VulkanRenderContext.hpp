#pragma once

#include "vk_mem_alloc.h"

namespace CZ {

struct VulkanRenderContext {
    VkInstance Instance             = VK_NULL_HANDLE;
    VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
    VkDevice Device                 = VK_NULL_HANDLE;
    VmaAllocator VmaAllocator       = VK_NULL_HANDLE;
};

} // namespace CZ