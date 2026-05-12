#include "VulkanImGuiRenderer.h"

#include "../../../Backend/Vulkan/VulkanCommandBufferObj.h"
#include "../../../Backend/Vulkan/VulkanGraphicsContextObj.h"
#include "../../../Backend/Vulkan/VulkanUtils.h"
#include "Core/Log/LogMacros.h"

#include <Runtime/App/Application.h>
#include <Runtime/RHI/RHIAPI.h>

// #include <stdio.h>  // printf, fprintf
// #include <stdlib.h> // abort

using namespace CZ;

DEFINE_LOG_CATEGORY_STATIC(LogVulkanImGuiRenderer, Info);

static void CheckVKResult(VkResult err) {
    if (err == VK_SUCCESS) return;

    if (err < 0) {
        // [Note] Negative values are typically unrecoverable errors in Vulkan
        CZ_CORE_LOG(Fatal, "[Vulkan] Error: VkResult = {}", VulkanUtils::VkResultToString(err));

    } else {
        // [Note] Log the error code
        CZ_CORE_LOG(Error, "[Vulkan] Error: VkResult = {}", VulkanUtils::VkResultToString(err));
    }
}

VulkanImGuiRenderer::VulkanImGuiRenderer() {}

VulkanImGuiRenderer::~VulkanImGuiRenderer() {}

void VulkanImGuiRenderer::Init(ImGuiContext* ctx, SDL_Window* windowHandle) {
    ImGui::SetCurrentContext(ctx);

    // Setup Platform/Renderer backends

    auto GraphicsContext = RHIAPI::Get().GetGraphicsContext();
    // auto device         = RHIAPI::Get().GetDevice();
    auto swapchain       = GraphicsContext.GetSwapchain();

    auto vulkanCtxWrapper =
        static_cast<VulkanGraphicsContextObj*>(GraphicsContext.Unwrap())->GetVulkanContextWrapper();

    auto vkInstance             = vulkanCtxWrapper.Instance;
    auto vkDevice               = vulkanCtxWrapper.Device;
    auto vkPhysicalDevice       = vulkanCtxWrapper.PhysicalDevice;
    auto vkSwapchain            = vulkanCtxWrapper.Swapchain;
    auto vkQueue                = vulkanCtxWrapper.GraphicsQueue;
    auto vkGlobalDescriptorPool = vulkanCtxWrapper.GlobalDescriptorPool;

    static VkFormat colorFormats[1];
    colorFormats[0] = VulkanUtils::ToVkFormat(swapchain.GetImageFormat());

    ImGui_ImplSDL3_InitForVulkan(windowHandle);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.ApiVersion                = VK_API_VERSION_1_4; // Pass in your value of
    // VkApplicationInfo::apiVersion, otherwise will default to header version.
    init_info.Instance                  = vkInstance;
    init_info.PhysicalDevice            = vkPhysicalDevice;
    init_info.Device                    = vkDevice;
    init_info.QueueFamily               = vulkanCtxWrapper.GraphicsQueueIndex;
    init_info.Queue                     = vkQueue;
    // init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool            = vkGlobalDescriptorPool;
    // init_info.DescriptorPoolSize = 1000;
    init_info.MinImageCount             = 2;
    init_info.ImageCount                = swapchain.GetImageCount();

    VkPipelineRenderingCreateInfoKHR dynamic_rendering_info = {};
    dynamic_rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    dynamic_rendering_info.colorAttachmentCount    = 1;
    dynamic_rendering_info.pColorAttachmentFormats = colorFormats;
    dynamic_rendering_info.depthAttachmentFormat =
        VulkanUtils::ToVkFormat(swapchain.GetDepthFormat());

    init_info.UseDynamicRendering                          = true;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo = dynamic_rendering_info;
    // init_info.PipelineInfoMain.PipelineRenderingCreateInfo.sType =
    //     VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    // init_info.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    // init_info.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats =
    // colorFormats; init_info.PipelineInfoMain.PipelineRenderingCreateInfo.depthAttachmentFormat =
    //     static_cast<VkFormat>(swapchain->GetVKDepthFormat());
    init_info.PipelineInfoMain.MSAASamples                 = VK_SAMPLE_COUNT_1_BIT;
    // init_info.PipelineInfoMain.RenderPass = *swapchain->GetVKRenderPass();
    // init_info.PipelineInfoMain.Subpass = 0;
    // init_info.PipelineInfoForViewports = init_info.PipelineInfoMain;
    init_info.CheckVkResultFn                              = CheckVKResult;

    ImGui_ImplVulkan_Init(&init_info);
}

void VulkanImGuiRenderer::Shutdown() {
    auto GraphicsContext = RHIAPI::Get().GetGraphicsContext();

    auto device = GraphicsContext.GetDevice();

    device.WaitIdle();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void VulkanImGuiRenderer::NewFrame() {
    ImGui_ImplSDL3_NewFrame();
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();
}

void VulkanImGuiRenderer::Draw(ImDrawData* drawData, CommandList cmdList) {
    if (!drawData || drawData->TotalVtxCount == 0) return;

    auto vkCmdBuffer = static_cast<VulkanCommandBufferObj*>(cmdList.Unwrap())->GetVkCommandBuffer();

    ImGui_ImplVulkan_RenderDrawData(drawData, vkCmdBuffer);
}
