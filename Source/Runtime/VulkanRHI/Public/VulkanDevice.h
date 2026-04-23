#pragma once

#include "RHIDevice.h"

#include "VulkanRHIExport.h"

#include "vk_mem_alloc.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanDevice, Info);

enum class EDescriptorLayoutType {
    CombinedImageSampler, // Used for ImGui or basic texture mapping
    UniformBuffer,        // Used for Camera data or Object data
    StorageImage          // Used for Compute shaders
};

class CVulkanCommandPool;
class CVulkanCommandBuffer;
class CVulkanImage;
class CVulkanSampler;
class CVulkanTexture2D;
class CVulkanTextureCubemap;

struct DynamicState3Functions {
    PFN_vkCmdSetPolygonModeEXT vkCmdSetPolygonModeEXT{ nullptr };
    PFN_vkCmdSetCullModeEXT vkCmdSetCullModeEXT{ nullptr };
    PFN_vkCmdSetDepthBoundsTestEnableEXT vkCmdSetDepthBoundsTestEnableEXT{ nullptr };
    PFN_vkCmdSetDepthCompareOpEXT vkCmdSetDepthCompareOpEXT{ nullptr };
    PFN_vkCmdSetDepthTestEnableEXT vkCmdSetDepthTestEnableEXT{ nullptr };
    PFN_vkCmdSetDepthWriteEnableEXT vkCmdSetDepthWriteEnableEXT{ nullptr };
    PFN_vkCmdSetFrontFaceEXT vkCmdSetFrontFaceEXT{ nullptr };
    PFN_vkCmdSetPrimitiveRestartEnableEXT vkCmdSetPrimitiveRestartEnableEXT{ nullptr };
    PFN_vkCmdSetPrimitiveTopologyEXT vkCmdSetPrimitiveTopologyEXT{ nullptr };
    PFN_vkCmdSetRasterizerDiscardEnableEXT vkCmdSetRasterizerDiscardEnableEXT{ nullptr };
    PFN_vkCmdSetScissorWithCountEXT vkCmdSetScissorWithCountEXT{ nullptr };
    PFN_vkCmdSetStencilOpEXT vkCmdSetStencilOpEXT{ nullptr };
    PFN_vkCmdSetStencilTestEnableEXT vkCmdSetStencilTestEnableEXT{ nullptr };
    PFN_vkCmdSetViewportWithCountEXT vkCmdSetViewportWithCountEXT{ nullptr };
    PFN_vkCmdSetColorBlendEnableEXT vkCmdSetColorBlendEnableEXT{ nullptr };
    PFN_vkCmdSetColorBlendEquationEXT vkCmdSetColorBlendEquationEXT{ nullptr };
    PFN_vkCmdSetColorWriteMaskEXT vkCmdSetColorWriteMaskEXT{ nullptr };
    PFN_vkCmdSetDepthClampEnableEXT vkCmdSetDepthClampEnableEXT{ nullptr };
    PFN_vkCmdSetLogicOpEXT vkCmdSetLogicOpEXT{ nullptr };
    PFN_vkCmdSetPatchControlPointsEXT vkCmdSetPatchControlPointsEXT{ nullptr };
    PFN_vkCmdSetTessellationDomainOriginEXT vkCmdSetTessellationDomainOriginEXT{ nullptr };
};

class VULKAN_RHI_API CVulkanDevice : public IRHIDevice {
    friend class CVulkanAPI;

public:
    CVulkanDevice(const IRHIContext* ctx, const FDeviceSpecification& spec,
                  const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface);
    virtual ~CVulkanDevice();

    virtual void WaitIdle() override;
    virtual GPUProfiler GetProfiler() override;

private:
    virtual TRef<IRHICommandPool> CreateCommandPool(FCommandPoolSpecification& spec) override;
    virtual TScope<IRHIImage> CreateImage(const FImageSpecification& spec) override;
    virtual TRef<IRHISampler> CreateSampler(const FSamplerSpecification& spec) override;
    virtual TRef<IRHISetLayout> CreateSetLayout(const FRHISetLayoutDescription& desc) override;
    virtual TScope<IRHITexture> CreateTexture(const FTextureSpecification& spec) override;
    virtual TScope<IRHITexture> CreateTexture(const FTextureSpecification& spec,
                                              TScope<IRHIImage> ownedImage) override;
    virtual TScope<IRHITexture> CreateTexture(const FTextureSpecification& spec,
                                              IRHIImage* borrowedImage) override;
    virtual TScope<IRHITexture> CreateTexture(const FTextureSpecification& spec,
                                              FBuffer& data) override;
    virtual TRef<IRHIDescriptorSet>
        CreateDescriptorSet(TRef<IRHISetLayout> setLayout,
                            const std::vector<FDescriptorBinding>& bindings) override;

private:
    void PickPhysicalDevice(const vk::raii::Instance& instance);
    void CreateLogicalDevice(const vk::raii::SurfaceKHR& surface);
    void InitGlobalDescriptorPool();
    void LoadDynamicState3Functions();

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
    const vk::PhysicalDeviceProperties GetPhysicalDeviceProperties() const {
        return m_PhysicalDevice.getProperties();
    }
    const vk::Queue GetGraphicsQueue() const { return *m_GraphicsQueue; }
    const uint32 GetGraphicsQueueIndex() const { return m_GraphicsQueueIndex; }
    const vk::DescriptorPool GetGlobalDescriptorPool() const { return *m_GlobalDescriptorPool; }
    const DynamicState3Functions& GetDynamicState3Functions() const {
        return m_DynamicState3Functions;
    }

    bool IsExtensionSupported(const std::string& extensionName) const;

    vk::raii::DescriptorSet AllocateSetFromPool(vk::DescriptorSetLayout layout);

    VmaAllocator GetVmaAllocator() const { return m_VmaAllocator; }

private:
    std::vector<const char*> m_RequiredDeviceExtension = { vk::KHRSwapchainExtensionName };
    vk::raii::PhysicalDevice m_PhysicalDevice          = nullptr;
    vk::raii::Device m_LogicalDevice                   = nullptr;

    vk::raii::Queue m_GraphicsQueue = nullptr;
    vk::raii::Queue m_PresentQueue  = nullptr;
    vk::raii::Queue m_ComputeQueue  = nullptr;
    uint32 m_GraphicsQueueIndex;

    vk::raii::DescriptorPool m_GlobalDescriptorPool = nullptr;

    // Dedicated pool for one-time initialization/upload commands
    mutable TRef<CVulkanCommandPool> m_InternalTransientPool;

    DynamicState3Functions m_DynamicState3Functions;

    VmaAllocator m_VmaAllocator = nullptr;
};
