#pragma once

#include "RHIDescriptorSet.h"

#include "VulkanDevice.h"
#include "VulkanRHIExport.h"
#include "VulkanSetLayout.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanDescriptorSet, Info);

class VULKAN_RHI_API CVulkanDescriptorSet : public IRHIDescriptorSet {
public:
    CVulkanDescriptorSet(const WeakRef<IRHIDevice> device, TRef<IRHISetLayout> setLayout,
                         const std::vector<FDescriptorBinding>& bindings);
    virtual ~CVulkanDescriptorSet() = default;

    virtual void* GetRawHandle() const override { return (void*)GetVKHandle(); }

    const vk::DescriptorSet GetVKHandle() const { return *m_RAIIHandle; }

private:
    void Init();

private:
    vk::raii::DescriptorSet m_RAIIHandle = nullptr;

    uint32 m_Slot;
};