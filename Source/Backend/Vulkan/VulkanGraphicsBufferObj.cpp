#include "VulkanDeviceObj.hpp"
#include "VulkanGraphicsBufferObj.hpp"

#include <Core/Header/Enum.h>
#include <Runtime/RHI/GraphicsBuffer.hpp>
#include <Runtime/RHI/RHITypes.hpp>

namespace CZ {

/* ---------- Static helper functions ---------- */

static VkDeviceSize GetAlignment(VkPhysicalDevice physicalDevice, size_t minAlignment,
                                 BufferUsage usage) {
    if (minAlignment > 0) {
        return minAlignment;
    }

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevice, &props);

    if (HasFlag(usage, BufferUsage::UniformBuffer)) {
        return props.limits.minUniformBufferOffsetAlignment;
    } else if (HasFlag(usage, BufferUsage::StorageBuffer)) {
        return props.limits.minStorageBufferOffsetAlignment;
    } else if (HasFlag(usage, BufferUsage::AccelerationStructure)) {
        return 256; // typical requirement
    }
    return 1; // no alignment requirement
}

static VkBufferUsageFlags ConvertUsageFlags(BufferUsage usage) {
    VkBufferUsageFlags vkUsage = 0;
    if (HasFlag(usage, BufferUsage::TransferSrc)) vkUsage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (HasFlag(usage, BufferUsage::TransferDst)) vkUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (HasFlag(usage, BufferUsage::UniformTexelBuffer))
        vkUsage |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    if (HasFlag(usage, BufferUsage::StorageTexelBuffer))
        vkUsage |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    if (HasFlag(usage, BufferUsage::UniformBuffer)) vkUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (HasFlag(usage, BufferUsage::StorageBuffer)) vkUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (HasFlag(usage, BufferUsage::IndexBuffer)) vkUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (HasFlag(usage, BufferUsage::VertexBuffer)) vkUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (HasFlag(usage, BufferUsage::IndirectBuffer)) vkUsage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    if (HasFlag(usage, BufferUsage::ShaderDeviceAddress))
        vkUsage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    if (HasFlag(usage, BufferUsage::AccelerationStructure))
        vkUsage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
    return vkUsage;
}

/* ---------- Public API ---------- */

VulkanGraphicsBufferObj::~VulkanGraphicsBufferObj() {
    CZ_BACKEND_LOG(Trace, "Destroyed Buffer: {} {}", (void*)m_VkBuffer, m_Spec.Name);

    if (m_MappedData) {
        Unmap();
    }

    vmaDestroyBuffer(m_DeviceObj->GetVmaAllocator(), m_VkBuffer, m_VmaAllocation);
}

void* VulkanGraphicsBufferObj::Map(size_t offset, size_t size) {
    if (m_MappedData && m_IsPersistentMapping && offset == 0 &&
        (size == 0 || size == m_AlignedSize)) {
        return m_MappedData;
    }

    void* data   = nullptr;
    VkResult res = vmaMapMemory(m_DeviceObj->GetVmaAllocator(), m_VmaAllocation, &data);
    if (res != VK_SUCCESS) {
        CZ_BACKEND_LOG(Error, "Failed to map buffer memory: {}", (int)res);
        return nullptr;
    }
    return data;
}

void VulkanGraphicsBufferObj::Unmap() {
    if (m_IsPersistentMapping) return;

    vmaUnmapMemory(m_DeviceObj->GetVmaAllocator(), m_VmaAllocation);
    m_MappedData = nullptr;
}

void VulkanGraphicsBufferObj::SetData(const Buffer* data, size_t offset) {
    if (!data) return;
    size_t size = data->GetSize();
    if (size == 0) return;

    if (!HasFlag(m_Spec.MemoryType, MemoryType::HostVisible)) {
        CZ_BACKEND_LOG(Error, "Cannot set data on non-host-visible buffer");
        return;
    }

    m_Offset = offset;

    void* mapped = Map(offset, size);
    if (!mapped) return;

    memcpy(mapped, data->Data, size);

    if (!HasFlag(m_Spec.MemoryType, MemoryType::HostCoherent)) {
        vmaFlushAllocation(m_DeviceObj->GetVmaAllocator(), m_VmaAllocation, offset, size);
    }

    if (!m_IsPersistentMapping) {
        Unmap();
    }
}

VkResult VulkanGraphicsBufferObj::Init(const Buffer* initialData) {
    VkResult result;

    VkDevice logicalDevice          = m_DeviceObj->GetLogicalDevice();
    VkPhysicalDevice physicalDevice = m_DeviceObj->GetPhysicalDevice();

    /* Convert flags */
    VkBufferUsageFlags vkUsage = ConvertUsageFlags(m_Spec.Usage);

    /* Compute alignment and aligned size */
    VkDeviceSize alignment = GetAlignment(physicalDevice, m_Spec.MinAlignment, m_Spec.Usage);
    m_AlignedSize          = m_Spec.Size;
    if (alignment > 0 && (m_Spec.Size % alignment) != 0) {
        m_AlignedSize = ((m_Spec.Size + alignment - 1) / alignment) * alignment;
        CZ_BACKEND_LOG(Trace, "Aligned buffer size from {} to {}", m_Spec.Size,
                       (size_t)m_AlignedSize);
    }

    /* Create buffer */
    VkBufferCreateInfo bufferInfo = {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = m_AlignedSize,
        .usage       = vkUsage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VmaAllocationCreateInfo allocCreateInfo = {};
    if (HasFlag(m_Spec.MemoryType, MemoryType::HostVisible)) {
        // If the buffer needs CPU access, VMA will handle it automatically
        allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        // Request persistent mapping + sequential write optimization when host-visible
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                VMA_ALLOCATION_CREATE_MAPPED_BIT;
    } else {
        // GPU-local memory, e.g., for vertex or index buffers
        allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    }

    // result = vkCreateBuffer(logicalDevice, &bufferInfo, NULL, &m_VkBuffer);
    VmaAllocationInfo allocInfo = {};
    result = vmaCreateBuffer(m_DeviceObj->GetVmaAllocator(), &bufferInfo, &allocCreateInfo,
                             &m_VkBuffer, &m_VmaAllocation, &allocInfo);
    RETURN_ON_VULKAN_FAIL(result);

    /* Get memory requirements */
    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(logicalDevice, m_VkBuffer, &memReqs);

    /* Persistent mapping if host-visible */
    if (HasFlag(m_Spec.MemoryType, MemoryType::HostVisible)) {
        m_IsPersistentMapping = true;
        m_MappedData =
            allocInfo.pMappedData; // VMA guarantees this pointer remains valid until destruction
    }

    /* Upload initial data if provided */
    if (initialData != nullptr) {
        SetData(initialData, 0);
    }

    CZ_BACKEND_LOG(Info, "Created Vulkan buffer: {} (size: {}, usage: {}, memory: {}, address: {})",
                   m_Spec.Name.empty() ? "Unnamed" : m_Spec.Name, m_Spec.Size,
                   static_cast<uint32_t>(m_Spec.Usage), static_cast<uint32_t>(m_Spec.MemoryType),
                   (void*)m_VkBuffer);

    return result;
}

} // namespace CZ
