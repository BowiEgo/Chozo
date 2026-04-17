#pragma once

#include "RHIBuffer.h"
#include "VulkanDevice.h"

#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanBuffer, Info);

class VULKAN_RHI_API CVulkanBuffer : public IRHIBuffer {
public:
    CVulkanBuffer(const WeakRef<IRHIDevice>& device, const FBufferSpecification& spec);
    CVulkanBuffer(const WeakRef<IRHIDevice>& device, const FBufferSpecification& spec,
                  FBuffer& data);
    virtual ~CVulkanBuffer();

    // IRHIBuffer interface implementation
    virtual void* Map(size_t offset, size_t size) override;
    virtual void Unmap() override;
    virtual void SetData(FBuffer& data, size_t offset) override;

    // Vulkan-specific getters
    vk::Buffer GetVKBuffer() const { return m_Buffer; }
    vk::DeviceMemory GetVKMemory() const { return m_Memory; }
    vk::DeviceSize GetVKSize() const { return m_AlignedSize; }
    vk::DeviceAddress GetVKDeviceAddress() const;
    vk::DescriptorBufferInfo GetVKBufferInfo();

private:
    void CreateBuffer();
    uint32 FindMemoryType(uint32 typeFilter, vk::MemoryPropertyFlags properties);
    vk::DeviceSize GetAlignment() const;

private:
    vk::Buffer m_Buffer          = VK_NULL_HANDLE;
    vk::DeviceMemory m_Memory    = VK_NULL_HANDLE;
    vk::DeviceSize m_AlignedSize = 0;
    size_t m_Offset              = 0; // For non-persistent mapping

    void* m_MappedData         = nullptr;
    bool m_IsPersistentMapping = false;
};
