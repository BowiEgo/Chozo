#include "VulkanImGuiRenderer.h"

#include "RHIContext.h"
#include "VulkanAPI.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanImage.h"
#include "VulkanSampler.h"
#include "VulkanSwapchain.h"

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
                                                            IRHIContext* rhiContext) {
    return new CVulkanImGuiRenderer(window, rhiContext);
}
}

CVulkanImGuiRenderer::CVulkanImGuiRenderer(CWindow* window, IRHIContext* rhiContext)
    : IImGuiRenderer(window, rhiContext) {}

static void CheckVKResult(VkResult err) {
    if (err == VK_SUCCESS) return;

    // [Note] Log the error code
    fprintf(stderr, "[Vulkan] Error: VkResult = %d\n", err);

    if (err < 0) {
        // [Note] Negative values are typically unrecoverable errors in Vulkan
        abort();
    }
}

void CVulkanImGuiRenderer::Init(ImGuiContext* ctx) {
    ImGui::SetCurrentContext(ctx);

    auto device       = m_Context->GetDevice().As<CVulkanDevice>();
    auto swapchain    = m_Context->GetSwapchain().As<CVulkanSwapchain>();
    auto windowHandle = (GLFWwindow*)m_Window->GetWindowWrapper();

    static VkFormat colorFormats[1];
    colorFormats[0] = static_cast<VkFormat>(swapchain->GetVKImageFormat());

    // m_Window->InitImGui(ctx);
    ImGui_ImplGlfw_InitForVulkan(windowHandle, true);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.ApiVersion                = VK_API_VERSION_1_4; // Pass in your value of
    // VkApplicationInfo::apiVersion, otherwise will default to header version.
    init_info.Instance                  = CVulkanAPI::GetVKInstance();
    init_info.PhysicalDevice            = device->GetPhysicalDevice();
    init_info.Device                    = device->GetLogicalDevice();
    init_info.QueueFamily               = device->GetGraphicsQueueIndex();
    init_info.Queue                     = device->GetGraphicsQueue();
    // init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool            = device->GetGlobalDescriptorPool();
    // init_info.DescriptorPoolSize = 1000;
    init_info.MinImageCount             = 2;
    init_info.ImageCount                = swapchain->GetImageCount();

    VkPipelineRenderingCreateInfoKHR dynamic_rendering_info = {};
    dynamic_rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    dynamic_rendering_info.colorAttachmentCount    = 1;
    dynamic_rendering_info.pColorAttachmentFormats = colorFormats;
    dynamic_rendering_info.depthAttachmentFormat =
        static_cast<VkFormat>(swapchain->GetVKDepthFormat());

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

void CVulkanImGuiRenderer::Shutdown() {
    auto device = m_Context->GetDevice().As<CVulkanDevice>();

    device->WaitIdle();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    m_DefaultBlackTexture.reset();
    m_bShutdown = true;
}

void CVulkanImGuiRenderer::NewFrame() {
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();
}

void CVulkanImGuiRenderer::Draw(ImDrawData* drawData, const TRef<IRHICommandList>& cmdBuffer) {
    if (!drawData || drawData->TotalVtxCount == 0) return;

    auto vlkCmdBuffer     = cmdBuffer.As<CVulkanCommandBuffer>();
    vk::CommandBuffer cmd = vlkCmdBuffer->GetVKCommandBuffer();

    ImGui_ImplVulkan_RenderDrawData(drawData, cmd);
}

ImTextureID CVulkanImGuiRenderer::GetTextureIDForRHITexture(const IRHITexture* texture) {
    auto it = m_TextureIDCache.find(texture);
    if (it != m_TextureIDCache.end()) {
        return it->second;
    }

    if (!texture->IsValid()) texture = m_DefaultBlackTexture.get();
    // texture = m_DefaultBlackTexture.get();

    vk::ImageView imageView = static_cast<CVulkanImage*>(texture->GetImage())->GetVKView();
    vk::Sampler sampler     = texture->GetSampler().As<CVulkanSampler>()->GetVKHandle();

    // CZ_LOG(LogVulkanImGuiRenderer, Trace, "GetTextureIDForRHITexture: image {} with imageView
    // {}",
    //        (void*)static_cast<CVulkanImage*>(texture->GetImage())->GetVKHandle(),
    //        (void*)imageView);

    VkDescriptorSet descSet =
        ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // CZ_LOG(LogVulkanImGuiRenderer, Trace,
    //        "GetTextureIDForRHITexture: image {} with descriptor set {}",
    //        (void*)static_cast<CVulkanImage*>(texture->GetImage())->GetVKHandle(),
    //        (void*)descSet);
    ImTextureID id            = reinterpret_cast<ImTextureID>(descSet);
    m_TextureIDCache[texture] = id;
    return id;
}

void CVulkanImGuiRenderer::ReleaseTextureID(ImTextureID id) {
    if (m_bShutdown) return;
    CZ_LOG(LogVulkanImGuiRenderer, Trace, "Releasing ImGui Texture ID: {}", id);
    ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(id));
}