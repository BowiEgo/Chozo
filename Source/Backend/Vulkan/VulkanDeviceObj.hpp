#pragma once

#include <Runtime/RHI/Device.hpp>

#include <vulkan/vulkan_core.h>

#ifndef VMA_IMPLEMENTATION
typedef struct VmaAllocator_T* VmaAllocator;
#endif

namespace CZ {

#define MAX_DEVICE_EXTENSIONS 16

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

class VulkanGraphicsContextObj;

class VulkanDeviceObj : public DeviceObj {
public:
    explicit VulkanDeviceObj(const VulkanGraphicsContextObj* ctxObj,
                             const DeviceSpecification& spec)
        : DeviceObj(spec), m_GraphicContextObj(ctxObj) {}
    ~VulkanDeviceObj() override;

    static Result<VulkanDeviceObj*, VkResult> Create(const VulkanGraphicsContextObj* ctxObj,
                                                     const DeviceSpecification& spec) {
        if (!ctxObj)
            return Result<VulkanDeviceObj*, VkResult>::Error(VK_ERROR_INITIALIZATION_FAILED);

        auto* obj = CZ_NEW(MEMORY_USAGE_RENDER, VulkanDeviceObj, ctxObj, spec);
        if (!obj) return Result<VulkanDeviceObj*, VkResult>::Error(VK_ERROR_OUT_OF_HOST_MEMORY);

        VkResult res = obj->Init();
        if (res != VK_SUCCESS) {
            Delete(obj);
            return Result<VulkanDeviceObj*, VkResult>::Error(res);
        }

        return Result<VulkanDeviceObj*, VkResult>::Success(obj);
    }

    void WaitIdle() override;

    CommandPool CreateCommandPool(CommandPoolSpecification& spec) override;

    Sampler CreateSampler(const SamplerSpecification spec) override;

    FrameBuffer CreateFrameBuffer(const FrameBufferSpecification& spec) override;

    ShaderRes CreateShaderRes(const ShaderResSpecification& spec,
                              const std::vector<uint32_t>* binary) override;

    Pipeline CreatePipeline(const PipelineSpecification& spec,
                            const std::vector<ShaderRes>& shaders,
                            const ShaderReflection& reflection) override;

    SetLayout CreateSetLayout(const SetLayoutDescription& desc) override;

    DescriptorSet CreateDescriptorSet(SetLayout setLayout,
                                      std::vector<DescriptorBinding>& bindings) override;

    GraphicsBuffer CreateGraphicsBuffer(const GraphicsBufferSpecification& spec,
                                        const Buffer* initialData = nullptr) override;

    VkDevice GetLogicalDevice() const { return m_VkDevice; }

    VkPhysicalDevice GetPhysicalDevice() const { return m_VkPhysicalDevice; }

    uint32 GetGraphicsQueueIndex() const { return m_GraphicsQueueIndex; }

    VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }

    VkDescriptorPool GetGlobalDescriptorPool() const { return m_GlobalDescriptorPool; }

    bool IsExtensionSupported(const std::string& extensionName) const;

    VkDescriptorPool CreateDescriptorPool(uint32_t maxSets,
                                          const std::vector<VkDescriptorPoolSize>& poolSizes);

    VkDescriptorSet AllocateSetFromPool(VkDescriptorSetLayout layout) const;

    VmaAllocator GetVmaAllocator() const { return m_VmaAllocator; }

    DynamicState3Functions GetDynamicState3Functions() const { return m_DynamicState3Functions; }

private:
    VkResult Init();
    VkResult PickPhysicalDevice();
    VkResult CreateLogicalDevice();
    VkResult CreateVmaAllocator();
    void InitGlobalDescriptorPool();
    void LoadDynamicState3Functions();

    const VulkanGraphicsContextObj* m_GraphicContextObj;

    const char* m_RequiredDeviceExtensions[MAX_DEVICE_EXTENSIONS];
    uint32_t m_RequiredDeviceExtensionCount = 0;

    VkDevice m_VkDevice                 = VK_NULL_HANDLE;
    VkPhysicalDevice m_VkPhysicalDevice = VK_NULL_HANDLE;

    VkQueue m_GraphicsQueue       = VK_NULL_HANDLE;
    VkQueue m_PresentQueue        = VK_NULL_HANDLE;
    VkQueue m_ComputeQueue        = VK_NULL_HANDLE;
    uint32_t m_GraphicsQueueIndex = 0;

    VkDescriptorPool m_GlobalDescriptorPool = VK_NULL_HANDLE;

    DynamicState3Functions m_DynamicState3Functions;

    VmaAllocator m_VmaAllocator = nullptr;
};
} // namespace CZ
