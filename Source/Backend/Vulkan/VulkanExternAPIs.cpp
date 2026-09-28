#include "VulkanDeviceObj.hpp"
#include "VulkanGraphicsContextObj.hpp"
#include "VulkanSwapchainObj.hpp"
#include "VulkanUIBackend.hpp"

namespace CZ {

extern "C" {

GraphicsContextObj* CreateVulkanGraphicsContextObj(const GraphicsContextSpecification& spec) {
    return CZ_NEW(MEMORY_USAGE_RENDER, VulkanGraphicsContextObj, spec);
}

RHIAPIObj* CreateVulkanAPIObj(GraphicsContext ctx) {
    return CZ_NEW(MEMORY_USAGE_RENDER, VulkanAPIObj, ctx);
}

/// UI (ImGui) backend factory looked up through the dynamic library registry.
UIRenderBackendObj* CreateUIBackend() { return CZ_NEW(MEMORY_USAGE_RENDER, VulkanUIBackend); }
}
} // namespace CZ