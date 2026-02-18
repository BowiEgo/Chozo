#include "VulkanImGuiRenderer.h"

#include "VulkanRHI.h"
#include "VulkanRHICommandBuffer.h"
#include "VulkanRHIDevice.h"
#include "VulkanRHISwapchain.h"

#include "imgui_impl_glfw.h"
#define IM_VULKAN_HAS_DYNAMIC_RENDERING
#include "imgui_impl_vulkan.h"
#include <stdio.h>  // printf, fprintf
#include <stdlib.h> // abort
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

DEFINE_LOG_CATEGORY(LogVulkanImGuiRenderer);

extern "C" {
VULKAN_IM_GUI_API IImGuiRenderer* CreateVulkanImGuiRenderer(CWindow* window,
                                                            CGraphicsContext* context) {
    return new CVulkanImGuiRenderer(window, context);
}
}

CVulkanImGuiRenderer::CVulkanImGuiRenderer(CWindow* window, CGraphicsContext* context)
    : IImGuiRenderer(window, context) {}

static void CheckVKResult(VkResult err) {
    if (err == VK_SUCCESS)
        return;

    // [Note] Log the error code
    fprintf(stderr, "[Vulkan] Error: VkResult = %d\n", err);

    if (err < 0) {
        // [Note] Negative values are typically unrecoverable errors in Vulkan
        abort();
    }
}

void CVulkanImGuiRenderer::Init(ImGuiContext* ctx) {
    ImGui::SetCurrentContext(ctx);

    auto RHI = static_cast<CVulkanRHI*>(m_Context->GetRHI());
    auto device = RHI->GetDevice().As<CVulkanRHIDevice>();
    auto swapchain = RHI->GetSwapchain().As<CVulkanRHISwapchain>();
    auto windowHandle = (GLFWwindow*)m_Window->GetWindowWrapper();

    static VkFormat colorFormats[1];
    colorFormats[0] = static_cast<VkFormat>(swapchain->GetVKImageFormat());

    // m_Window->InitImGui(ctx);
    ImGui_ImplGlfw_InitForVulkan(windowHandle, true);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.ApiVersion = VK_API_VERSION_1_4; // Pass in your value of
    // VkApplicationInfo::apiVersion, otherwise will default to header version.
    init_info.Instance = *RHI->GetVKInstance();
    init_info.PhysicalDevice = *device->GetPhysicalDevice();
    init_info.Device = *device->GetLogicalDevice();
    init_info.QueueFamily = device->GetGraphicsQueueIndex();
    init_info.Queue = *device->GetGraphicsQueue();
    // init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = *device->GetGlobalDescriptorPool();
    // init_info.DescriptorPoolSize = 1000;
    init_info.MinImageCount = 2;
    init_info.ImageCount = swapchain->GetImageCount();

    VkPipelineRenderingCreateInfoKHR dynamic_rendering_info = {};
    dynamic_rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    dynamic_rendering_info.colorAttachmentCount = 1;
    dynamic_rendering_info.pColorAttachmentFormats = colorFormats;
    dynamic_rendering_info.depthAttachmentFormat =
        static_cast<VkFormat>(swapchain->GetVKDepthFormat());

    init_info.UseDynamicRendering = true;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo = dynamic_rendering_info;
    // init_info.PipelineInfoMain.PipelineRenderingCreateInfo.sType =
    //     VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    // init_info.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    // init_info.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats =
    // colorFormats; init_info.PipelineInfoMain.PipelineRenderingCreateInfo.depthAttachmentFormat =
    //     static_cast<VkFormat>(swapchain->GetVKDepthFormat());
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    // init_info.PipelineInfoMain.RenderPass = *swapchain->GetVKRenderPass();
    // init_info.PipelineInfoMain.Subpass = 0;
    // init_info.PipelineInfoForViewports = init_info.PipelineInfoMain;
    init_info.CheckVkResultFn = CheckVKResult;

    ImGui_ImplVulkan_Init(&init_info);
}

void CVulkanImGuiRenderer::Shutdown() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void CVulkanImGuiRenderer::NewFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    // io.WantCaptureMouse = true;
    // CZ_LOG(LogVulkanImGuiRenderer, Info, "WantCaptureMouse: {}", io.WantCaptureMouse);
    // CZ_LOG(LogVulkanImGuiRenderer, Info, "Mouse Pos: ({}, {}) | MouseDown[0]: {}", io.MousePos.x,
    //        io.MousePos.y, io.MouseDown[0]);
}

void CVulkanImGuiRenderer::Render(ImDrawData* drawData, const TRef<IRHICommandBuffer> cmdBuffer) {
    if (!drawData || drawData->TotalVtxCount == 0)
        return;

    auto vlkCmdBuffer = cmdBuffer.As<CVulkanRHICommandBuffer>();
    VkCommandBuffer cmd = *vlkCmdBuffer->GetVKCommandBuffer();

    ImGui_ImplVulkan_RenderDrawData(drawData, cmd);
}
