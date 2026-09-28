#pragma once

#include <Runtime/RHI/Pipeline.hpp>
#include <Runtime/RHI/ShaderRes.hpp>

namespace CZ {

class VulkanDeviceObj;

class VulkanPipelineObj : public PipelineObj {
public:
    VulkanPipelineObj(VulkanDeviceObj* deviceObj, const PipelineSpecification& spec);
    ~VulkanPipelineObj() override;

    static Scope<VulkanPipelineObj> Create(VulkanDeviceObj* deviceObj,
                                           const PipelineSpecification& spec,
                                           const std::vector<ShaderRes>& shaderRes,
                                           const ShaderReflection& reflection) {
        if (!deviceObj) return nullptr;

        auto obj = CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, VulkanPipelineObj, deviceObj, spec);

        VkResult res = obj->Init(shaderRes, reflection);
        if (res != VK_SUCCESS) return nullptr;

        return obj;
    }

    VkPipeline GetVKPipeline() const { return m_VkPipeline; }
    VkPipelineLayout GetVKPipelineLayout() const { return m_VkPipelineLayout; }

private:
    VkResult Init(const std::vector<ShaderRes>& shaders, const ShaderReflection& reflection);

private:
    VulkanDeviceObj* m_DeviceObj;

    VkPipelineLayout m_VkPipelineLayout = nullptr;
    VkPipeline m_VkPipeline             = nullptr;
};

} // namespace CZ