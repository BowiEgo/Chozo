#pragma once

#include "RHIDevice.h"

#include "VulkanRHIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanDevice, Info);

enum class EDescriptorLayoutType {
    CombinedImageSampler, // Used for ImGui or basic texture mapping
    UniformBuffer,        // Used for Camera data or Object data
    StorageImage          // Used for Compute shaders
};

class CVulkanCommandPool;
class CVulkanCommandBuffer;

class VULKAN_RHI_API CVulkanDevice : public IRHIDevice {
public:
    CVulkanDevice(const IRHIContext* ctx, const FDeviceSpecification& spec,
                  const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface);
    virtual ~CVulkanDevice();

    virtual void WaitIdle() override;
    virtual TRef<IRHICommandPool> CreateCommandPool(FCommandPoolSpecification& spec) override;

private:
    void PickPhysicalDevice(const vk::raii::Instance& instance);
    void CreateLogicalDevice(const vk::raii::SurfaceKHR& surface);
    void InitGlobalDescriptorPool();

public:
    void Init();
    void InitInternalResources();
    vk::raii::DescriptorPool
        CreateDescriptorPool(uint32 maxSets, const std::vector<vk::DescriptorPoolSize>& poolSizes);
    uint32 FindMemoryType(uint32 typeFilter, vk::MemoryPropertyFlags properties) const;
    void CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                      vk::MemoryPropertyFlags properties, vk::Buffer& buffer,
                      vk::DeviceMemory& bufferMemory);
    TRef<CVulkanCommandBuffer> BeginSingleTimeCommands() const;
    void EndSingleTimeCommands(TRef<CVulkanCommandBuffer> cmdBuffer) const;

    const vk::PhysicalDevice GetPhysicalDevice() const { return *m_PhysicalDevice; }
    const vk::Device GetLogicalDevice() const { return *m_LogicalDevice; }
    const vk::raii::PhysicalDevice& GetRAIIPhysicalDevice() const { return m_PhysicalDevice; }
    const vk::raii::Device& GetRAIILogicalDevice() const { return m_LogicalDevice; }
    const vk::Queue GetGraphicsQueue() const { return *m_GraphicsQueue; }
    const uint32 GetGraphicsQueueIndex() const { return m_GraphicsQueueIndex; }
    const vk::DescriptorPool GetGlobalDescriptorPool() const { return *m_GlobalDescriptorPool; }
    vk::DescriptorSetLayout GetDescriptorSetLayout(EDescriptorLayoutType layoutType);

    bool IsExtensionSupported(const std::string& extensionName) const;

private:
    std::vector<const char*> m_RequiredDeviceExtension = { vk::KHRSwapchainExtensionName };
    vk::raii::PhysicalDevice m_PhysicalDevice = nullptr;
    vk::raii::Device m_LogicalDevice = nullptr;

    vk::raii::Queue m_GraphicsQueue = nullptr;
    vk::raii::Queue m_PresentQueue = nullptr;
    vk::raii::Queue m_ComputeQueue = nullptr;
    uint32 m_GraphicsQueueIndex;

    vk::raii::DescriptorPool m_GlobalDescriptorPool = nullptr;
    std::unordered_map<EDescriptorLayoutType, vk::DescriptorSetLayout> m_LayoutCache;

    // Dedicated pool for one-time initialization/upload commands
    mutable TRef<CVulkanCommandPool> m_InternalTransientPool;
};
