#include <Runtime/RHI/Device.h>

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

class VulkanDeviceObj : public DeviceObj {
public:
    explicit VulkanDeviceObj(const DeviceSpecification& spec);
    ~VulkanDeviceObj() override;

    void WaitIdle() override;

    VkDevice GetLogicalDevice() const { return m_VkDevice; }
    VkPhysicalDevice GetPhysicalDevice() const { return m_VkPhysicalDevice; }
    bool IsExtensionSupported(const std::string& extensionName) const;
    VkDescriptorPool CreateDescriptorPool(uint32_t maxSets,
                                          const std::vector<VkDescriptorPoolSize>& poolSizes);
    VmaAllocator GetVmaAllocator() const { return m_VmaAllocator; }

private:
    void PickPhysicalDevice(GraphicContext context);
    void CreateLogicalDevice(GraphicContext context);
    void CreateVmaAllocator(GraphicContext context);
    void InitGlobalDescriptorPool();
    void LoadDynamicState3Functions();

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
