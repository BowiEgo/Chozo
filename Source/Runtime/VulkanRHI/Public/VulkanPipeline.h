#pragma once

#include "RHIPipeline.h"

#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanPipeline, Info);

class CVulkanDevice;

class VULKAN_RHI_API CVulkanPipeline : public IRHIPipeline {
    friend class CVulkan;
    friend class CVulkanCommandBuffer;

public:
    CVulkanPipeline(const FPipelineSpecification& spec, const TRef<CVulkanDevice>& device);
    virtual ~CVulkanPipeline();

    vk::DescriptorSetLayout GetSetLayout(uint32_t set) {
        if (set < m_DescriptorSetLayouts.size()) return m_DescriptorSetLayouts[set];
        return nullptr;
    }

private:
    void Init();
    const vk::Pipeline GetVKPipeline() const { return *m_Pipeline; }
    const vk::raii::Pipeline& GetRAIIPipeline() const { return m_Pipeline; }
    const vk::PipelineLayout GetPipelineLayout() const { return *m_PipelineLayout; }
    const vk::raii::PipelineLayout& GetRAIIPipelineLayout() const { return m_PipelineLayout; }

private:
    WeakRef<CVulkanDevice> m_Device;

    vk::raii::PipelineLayout m_PipelineLayout = nullptr;
    vk::raii::Pipeline m_Pipeline             = nullptr;
    std::vector<vk::DescriptorSetLayout> m_DescriptorSetLayouts;
};
