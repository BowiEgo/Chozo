#pragma once

#include <Runtime/RHI/Pipeline.hpp>
#include <Runtime/RHI/ShaderRes.hpp>
#include <vector>

namespace CZ {

class VulkanDeviceObj;

class VulkanPipelineObj : public PipelineObj {
public:
    VulkanPipelineObj(VulkanDeviceObj* deviceObj, const PipelineSpecification& spec,
                      const std::vector<ShaderRes>& shaderRes, const ShaderReflection& reflection);
    ~VulkanPipelineObj() override;

private:
    VkResult Init();
    VkPipeline GetVKPipeline() const { return m_VkPipeline; }
    VkPipelineLayout GetPipelineLayout() const { return m_VkPipelineLayout; }

private:
    VulkanDeviceObj* m_DeviceObj;
    const std::vector<ShaderRes>& m_Shaders;
    const ShaderReflection& m_Reflection;

    VkPipelineLayout m_VkPipelineLayout = nullptr;
    VkPipeline m_VkPipeline             = nullptr;
};

} // namespace CZ