#pragma once

#include <Runtime/RHI/Pipeline.hpp>
#include <Runtime/RHI/ShaderRes.hpp>
#include <vector>

namespace CZ {

class VulkanDeviceObj;

class VulkanPipelineObj : public PipelineObj {
public:
    VulkanPipelineObj(VulkanDeviceObj* deviceObj, const PipelineSpecification& spec);
    ~VulkanPipelineObj() override;

    static Result<VulkanPipelineObj*, VkResult> Create(VulkanDeviceObj* deviceObj,
                                                       const PipelineSpecification& spec,
                                                       const std::vector<ShaderRes>& shaderRes,
                                                       const ShaderReflection& reflection) {
        if (!deviceObj)
            return Result<VulkanPipelineObj*, VkResult>::Error(VK_ERROR_INITIALIZATION_FAILED);

        auto* obj = CZ_NEW(MEMORY_USAGE_RENDER, VulkanPipelineObj, deviceObj, spec);
        if (!obj) return Result<VulkanPipelineObj*, VkResult>::Error(VK_ERROR_OUT_OF_HOST_MEMORY);

        VkResult res = obj->Init(shaderRes, reflection);
        if (res != VK_SUCCESS) {
            Delete(obj);
            return Result<VulkanPipelineObj*, VkResult>::Error(res);
        }

        return Result<VulkanPipelineObj*, VkResult>::Success(obj);
    }

    VkPipeline GetVKPipeline() const { return m_VkPipeline; }
    VkPipelineLayout GetPipelineLayout() const { return m_VkPipelineLayout; }

private:
    VkResult Init(const std::vector<ShaderRes>& shaders, const ShaderReflection& reflection);

private:
    VulkanDeviceObj* m_DeviceObj;

    VkPipelineLayout m_VkPipelineLayout = nullptr;
    VkPipeline m_VkPipeline             = nullptr;
};

} // namespace CZ