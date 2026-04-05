#pragma once

#include "RHIDescriptorSet.h"

#include "VulkanDevice.h"
#include "VulkanRHIExport.h"
#include "VulkanSetLayout.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanDescriptorSet, Info);

class VULKAN_RHI_API CVulkanDescriptorSet : public IRHIDescriptorSet {
public:
    CVulkanDescriptorSet(const WeakRef<IRHIDevice> device, const FTextureDescriptorInfo& info,
                         TRef<IRHISetLayout> setLayout, uint32 bindingSlot);
    virtual ~CVulkanDescriptorSet() = default;

    virtual void* GetRawHandle() const override { return (void*)GetVKDescriptorSet(); }

    const vk::DescriptorSet GetVKDescriptorSet() const { return *m_RAIIHandle; }

private:
    void Init();

private:
    vk::raii::DescriptorSet m_RAIIHandle = nullptr;

    FTextureDescriptorInfo m_Info;
    TRef<CVulkanSetLayout> m_Layout;
    uint32 m_Slot;
};