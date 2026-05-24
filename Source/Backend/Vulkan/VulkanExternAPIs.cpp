#include "VulkanDeviceObj.hpp"
#include "VulkanGraphicsContextObj.hpp"
#include "VulkanSwapchainObj.hpp"

namespace CZ {

extern "C" {

GraphicsContextObj* CreateVulkanGraphicsContextObj(const GraphicsContextSpecification& spec) {
    return CZ_NEW(MEMORY_USAGE_RENDER, VulkanGraphicsContextObj, spec);
}

RHIAPIObj* CreateVulkanAPIObj(GraphicsContext ctx) {
    return CZ_NEW(MEMORY_USAGE_RENDER, VulkanAPIObj, ctx);
}
}
} // namespace CZ