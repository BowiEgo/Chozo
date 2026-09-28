#pragma once
#include <cstddef>

#include <Runtime/RHI/GraphicsBuffer.hpp>

namespace CZ {

class VulkanDeviceObj;

class VulkanGraphicsBufferObj : public GraphicsBufferObj {
public:
    VulkanGraphicsBufferObj(const VulkanDeviceObj* deviceObj,
                            const GraphicsBufferSpecification& spec)
        : GraphicsBufferObj(spec), m_DeviceObj(deviceObj) {}
    ~VulkanGraphicsBufferObj() override;

    static Scope<VulkanGraphicsBufferObj> Create(VulkanDeviceObj* deviceObj,
                                                 const GraphicsBufferSpecification& spec,
                                                 const Buffer* initialData) {
        if (!deviceObj) return nullptr;

        auto obj = CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, VulkanGraphicsBufferObj, deviceObj, spec);

        VkResult res = obj->Init(initialData);
        if (res != VK_SUCCESS) return nullptr;

        return obj;
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
