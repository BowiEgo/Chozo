#include "VulkanBuffer.h"

DEFINE_LOG_CATEGORY(LogVulkanBuffer);

CVulkanBuffer::CVulkanBuffer(const WeakRef<IRHIDevice>& device, const FBufferSpecification& spec)
    : IRHIBuffer(device, spec) {
    CreateBuffer();

    CZ_LOG(LogVulkanBuffer, Info,
           "Created Vulkan buffer: {} (size: {}, usage: {}, memory: {}, address: {})",
           spec.Name.empty() ? "Unnamed" : spec.Name, spec.Size, static_cast<uint32_t>(spec.Usage),
           static_cast<uint32_t>(spec.MemoryType), (void*)m_Buffer);
}

CVulkanBuffer::CVulkanBuffer(const WeakRef<IRHIDevice>& device, const FBufferSpecification& spec,
                             FBuffer& data)
    : IRHIBuffer(device, spec) {
    CreateBuffer();
    SetData(data, 0);

    CZ_LOG(LogVulkanBuffer, Info,
           "Created Vulkan buffer: {} (size: {}, usage: {}, memory: {}, address: {})",
           spec.Name.empty() ? "Unnamed" : spec.Name, spec.Size, static_cast<uint32_t>(spec.Usage),
           static_cast<uint32_t>(spec.MemoryType), (void*)m_Buffer);
}

CVulkanBuffer::~CVulkanBuffer() {
    CZ_LOG(LogVulkanBuffer, Trace, "Destroyed Buffer: {} {}", (void*)m_Buffer, m_Spec.Name);
    auto device = m_Device.lock().As<CVulkanDevice>();
    if (!device) return;

    if (m_MappedData) {
        Unmap();
    }

    vk::Device vkDevice     = device->GetLogicalDevice();
    vk::Buffer buffer       = m_Buffer;
    vk::DeviceMemory memory = m_Memory;

    // Defer destruction to next frame (if device supports it)
    device->EnqueueCleanup([vkDevice, buffer, memory]() {
        if (buffer) vkDevice.destroyBuffer(buffer);
        if (memory) vkDevice.freeMemory(memory);
    });

    CZ_LOG(LogVulkanBuffer, Trace, "Vulkan buffer queued for destruction");
}

void CVulkanBuffer::CreateBuffer() {
    auto device = m_Device.lock().As<CVulkanDevice>();
    if (!device) {
        CZ_LOG(LogVulkanBuffer, Error, "Device is invalid");
        return;
    }

    vk::Device vkDevice = device->GetLogicalDevice();

    // Convert usage flags
    vk::BufferUsageFlags vkUsage = vk::BufferUsageFlags();

    if (HasFlag(m_Spec.Usage, EBufferUsage::TransferSrc))
        vkUsage |= vk::BufferUsageFlagBits::eTransferSrc;
    if (HasFlag(m_Spec.Usage, EBufferUsage::TransferDst))
        vkUsage |= vk::BufferUsageFlagBits::eTransferDst;
    if (HasFlag(m_Spec.Usage, EBufferUsage::UniformTexelBuffer))
        vkUsage |= vk::BufferUsageFlagBits::eUniformTexelBuffer;
    if (HasFlag(m_Spec.Usage, EBufferUsage::StorageTexelBuffer))
        vkUsage |= vk::BufferUsageFlagBits::eStorageTexelBuffer;
    if (HasFlag(m_Spec.Usage, EBufferUsage::UniformBuffer))
        vkUsage |= vk::BufferUsageFlagBits::eUniformBuffer;
    if (HasFlag(m_Spec.Usage, EBufferUsage::StorageBuffer))
        vkUsage |= vk::BufferUsageFlagBits::eStorageBuffer;
    if (HasFlag(m_Spec.Usage, EBufferUsage::IndexBuffer))
        vkUsage |= vk::BufferUsageFlagBits::eIndexBuffer;
    if (HasFlag(m_Spec.Usage, EBufferUsage::VertexBuffer))
        vkUsage |= vk::BufferUsageFlagBits::eVertexBuffer;
    if (HasFlag(m_Spec.Usage, EBufferUsage::IndirectBuffer))
        vkUsage |= vk::BufferUsageFlagBits::eIndirectBuffer;
    if (HasFlag(m_Spec.Usage, EBufferUsage::ShaderDeviceAddress))
        vkUsage |= vk::BufferUsageFlagBits::eShaderDeviceAddress;
    if (HasFlag(m_Spec.Usage, EBufferUsage::AccelerationStructure))
        vkUsage |= vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;

    // Get alignment requirements
    vk::DeviceSize alignment = GetAlignment();
    m_AlignedSize            = m_Spec.Size;

    if (alignment > 0 && m_Spec.Size % alignment != 0) {
        m_AlignedSize = ((m_Spec.Size + alignment - 1) / alignment) * alignment;
        CZ_LOG(LogVulkanBuffer, Trace, "Aligned buffer size from {} to {}", m_Spec.Size,
               m_AlignedSize);
    }

    // Create buffer
    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.setSize(m_AlignedSize).setUsage(vkUsage).setSharingMode(vk::SharingMode::eExclusive);

    try {
        m_Buffer = vkDevice.createBuffer(bufferInfo);
    } catch (const vk::SystemError& err) {
        CZ_LOG(LogVulkanBuffer, Error, "Failed to create buffer: {}", err.what());
        return;
    }

    // Get memory requirements
    vk::MemoryRequirements memReqs = vkDevice.getBufferMemoryRequirements(m_Buffer);

    // Convert memory type flags
    vk::MemoryPropertyFlags vkMemoryFlags = vk::MemoryPropertyFlags();

    if (HasFlag(m_Spec.MemoryType, EMemoryType::DeviceLocal))
        vkMemoryFlags |= vk::MemoryPropertyFlagBits::eDeviceLocal;
    if (HasFlag(m_Spec.MemoryType, EMemoryType::HostVisible))
        vkMemoryFlags |= vk::MemoryPropertyFlagBits::eHostVisible;
    if (HasFlag(m_Spec.MemoryType, EMemoryType::HostCoherent))
        vkMemoryFlags |= vk::MemoryPropertyFlagBits::eHostCoherent;
    if (HasFlag(m_Spec.MemoryType, EMemoryType::HostCached))
        vkMemoryFlags |= vk::MemoryPropertyFlagBits::eHostCached;
    if (HasFlag(m_Spec.MemoryType, EMemoryType::LazilyAllocated))
        vkMemoryFlags |= vk::MemoryPropertyFlagBits::eLazilyAllocated;
    if (HasFlag(m_Spec.MemoryType, EMemoryType::Protected))
        vkMemoryFlags |= vk::MemoryPropertyFlagBits::eProtected;

    // Allocate memory
    uint32_t memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, vkMemoryFlags);

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.setAllocationSize(memReqs.size).setMemoryTypeIndex(memoryTypeIndex);

    try {
        m_Memory = vkDevice.allocateMemory(allocInfo);
    } catch (const vk::SystemError& err) {
        CZ_LOG(LogVulkanBuffer, Error, "Failed to allocate buffer memory: {}", err.what());
        vkDevice.destroyBuffer(m_Buffer);
        m_Buffer = VK_NULL_HANDLE;
        return;
    }

    // Bind buffer to memory
    vkDevice.bindBufferMemory(m_Buffer, m_Memory, 0);

    // Persistent mapping if needed
    if (HasFlag(m_Spec.MemoryType, EMemoryType::HostVisible)) {
        m_IsPersistentMapping = true;
        m_MappedData          = Map(0, m_AlignedSize);
    }

    // Set debug name
    if (!m_Spec.Name.empty()) {
        // ChozoUtils::Vulkan::SetDebugObjectName(vkDevice, m_Buffer, m_Spec.Name);
    }
}

void* CVulkanBuffer::Map(size_t offset, size_t size) {
    if (m_MappedData && offset == 0 && size == m_AlignedSize) {
        return m_MappedData; // Already mapped persistently
    }

    auto device = m_Device.lock().As<CVulkanDevice>();
    if (!device) {
        CZ_LOG(LogVulkanBuffer, Error, "Device is invalid during Map");
        return nullptr;
    }

    vk::Device vkDevice = device->GetLogicalDevice();

    size_t mapSize = (size == 0) ? m_AlignedSize : size;

    void* data;
    try {
        data = vkDevice.mapMemory(m_Memory, offset, mapSize, {});
    } catch (const vk::SystemError& err) {
        CZ_LOG(LogVulkanBuffer, Error, "Failed to map memory: {}", err.what());
        return nullptr;
    }

    return data;
}

void CVulkanBuffer::Unmap() {
    if (m_IsPersistentMapping) {
        return; // Keep persistent mapping
    }

    auto device = m_Device.lock().As<CVulkanDevice>();
    if (!device) return;

    vk::Device vkDevice = device->GetLogicalDevice();
    vkDevice.unmapMemory(m_Memory);
    m_MappedData = nullptr;
}

void CVulkanBuffer::SetData(FBuffer& data, size_t offset) {
    size_t size = data.GetSize();
    if (!data.Data || size == 0) return;

    if (!HasFlag(m_Spec.MemoryType, EMemoryType::HostVisible)) {
        CZ_LOG(LogVulkanBuffer, Error, "Cannot set data on non-host-visible buffer");
        return;
    }

    m_Offset = offset;

    void* mapped = Map(offset, size);
    if (!mapped) return;

    memcpy(mapped, data.Data, size);

    if (!HasFlag(m_Spec.MemoryType, EMemoryType::HostCoherent)) {
        // Flush memory range
        auto device = m_Device.lock().As<CVulkanDevice>();
        if (device) {
            vk::Device vkDevice = device->GetLogicalDevice();
            vk::MappedMemoryRange range;
            range.setMemory(m_Memory).setOffset(offset).setSize(size);
            vkDevice.flushMappedMemoryRanges({ range });
        }
    }

    if (!m_IsPersistentMapping) {
        Unmap();
    }
}

vk::DeviceAddress CVulkanBuffer::GetVKDeviceAddress() const {
    if (!HasFlag(m_Spec.Usage, EBufferUsage::ShaderDeviceAddress)) {
        return 0;
    }

    auto device = m_Device.lock().As<CVulkanDevice>();
    if (!device) return 0;

    vk::Device vkDevice = device->GetLogicalDevice();

    vk::BufferDeviceAddressInfo addressInfo{};
    addressInfo.setBuffer(m_Buffer);

    return vkDevice.getBufferAddress(addressInfo);
}

uint32_t CVulkanBuffer::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    auto device = m_Device.lock().As<CVulkanDevice>();
    if (!device) return ~0u;

    vk::PhysicalDevice physicalDevice                = device->GetPhysicalDevice();
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    CZ_LOG(LogVulkanBuffer, Error, "Failed to find suitable memory type");
    return ~0u;
}

vk::DeviceSize CVulkanBuffer::GetAlignment() const {
    auto device = m_Device.lock().As<CVulkanDevice>();
    if (!device) return 0;

    if (m_Spec.MinAlignment > 0) {
        return m_Spec.MinAlignment;
    }

    // Get device limits
    vk::PhysicalDeviceProperties props = device->GetPhysicalDeviceProperties();

    if (HasFlag(m_Spec.Usage, EBufferUsage::UniformBuffer)) {
        return props.limits.minUniformBufferOffsetAlignment;
    } else if (HasFlag(m_Spec.Usage, EBufferUsage::StorageBuffer)) {
        return props.limits.minStorageBufferOffsetAlignment;
    } else if (HasFlag(m_Spec.Usage, EBufferUsage::AccelerationStructure)) {
        return 256; // Typical requirement for acceleration structures
    }

    return 1; // No alignment requirement
}

vk::DescriptorBufferInfo CVulkanBuffer::GetVKBufferInfo() {
    vk::DescriptorBufferInfo bufferInfo{};
    bufferInfo.setBuffer(m_Buffer);
    bufferInfo.setOffset(m_Offset);
    bufferInfo.setRange(m_AlignedSize);
    return bufferInfo;
}