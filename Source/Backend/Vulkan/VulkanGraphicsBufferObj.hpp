#pragma once

#include <Runtime/RHI/GraphicsBuffer.hpp>

namespace CZ {

class VulkanDeviceObj;

class VulkanGraphicsBufferObj : public GraphicsBufferObj {
public:
    VulkanGraphicsBufferObj(const VulkanDeviceObj* deviceObj,
                            const GraphicsBufferSpecification& spec)
        : GraphicsBufferObj(spec), m_DeviceObj(deviceObj) {}
    ~VulkanGraphicsBufferObj() override;

    static Result<VulkanGraphicsBufferObj*, VkResult>
        Create(VulkanDeviceObj* deviceObj, const GraphicsBufferSpecification& spec,
               const Buffer* initialData) {
        if (!deviceObj)
            return Result<VulkanGraphicsBufferObj*, VkResult>::Error(
                VK_ERROR_INITIALIZATION_FAILED);

        auto* obj = CZ_NEW(MEMORY_USAGE_RENDER, VulkanGraphicsBufferObj, deviceObj, spec);
        if (!obj)
            return Result<VulkanGraphicsBufferObj*, VkResult>::Error(VK_ERROR_OUT_OF_HOST_MEMORY);

        VkResult res = obj->Init(initialData);
        if (res != VK_SUCCESS) {
            Delete(obj);
            return Result<VulkanGraphicsBufferObj*, VkResult>::Error(res);
        }

        return Result<VulkanGraphicsBufferObj*, VkResult>::Success(obj);
    }

    virtual void* Map(size_t offset, size_t size) override;
    virtual void Unmap() override;
    virtual void SetData(const Buffer* data, size_t offset) override;

    // Vulkan-specific getters
    VkBuffer GetVKBuffer() const { return m_VkBuffer; }
    VkDeviceSize GetVKSize() const { return m_AlignedSize; }
    VkDeviceAddress GetVKDeviceAddress() const;
    VkDescriptorBufferInfo GetVKBufferInfo();

private:
    VkResult Init(const Buffer* initialData);

    const VulkanDeviceObj* m_DeviceObj;

    VmaAllocation m_VmaAllocation = VK_NULL_HANDLE;

    VkBuffer m_VkBuffer        = VK_NULL_HANDLE;
    VkDeviceSize m_AlignedSize = 0;
    size_t m_Offset            = 0; // For non-persistent mapping

    void* m_MappedData         = nullptr;
    bool m_IsPersistentMapping = false;
};

} // namespace CZ
