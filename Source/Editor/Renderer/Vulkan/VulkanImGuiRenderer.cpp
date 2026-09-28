#include "VulkanImGuiRenderer.hpp"

// #include "../../../Backend/Vulkan/VulkanPCH.h"
#include "../../../Backend/Vulkan/VulkanUtils.hpp"

#include "../../../Backend/Vulkan/VulkanCommandBufferObj.hpp"
#include "../../../Backend/Vulkan/VulkanGraphicsContextObj.hpp"
#include "../../../Backend/Vulkan/VulkanImageObj.hpp"
#include "../../../Backend/Vulkan/VulkanSamplerObj.hpp"
#include "../../../Backend/Vulkan/VulkanUtils.hpp"

#include <Runtime/App/Application.hpp>
#include <Runtime/RHI/RHIAPI.hpp>

// #include <stdio.h>  // printf, fprintf
// #include <stdlib.h> // abort

using namespace CZ;

static void CheckVKResult(VkResult err) {
    if (err == VK_SUCCESS) return;

    if (err < 0) {
        // [Note] Negative values are typically unrecoverable errors in Vulkan
        CZ_EDITOR_LOG(Fatal, "[Vulkan] Error: VkResult = {}", VulkanUtils::VkResultToString(err));

    } else {
        // [Note] Log the error code
        CZ_EDITOR_LOG(Error, "[Vulkan] Error: VkResult = {}", VulkanUtils::VkResultToString(err));
    }
}

VulkanImGuiRenderer::VulkanImGuiRenderer() {}

VulkanImGuiRenderer::~VulkanImGuiRenderer() {}

void VulkanImGuiRenderer::Init(ImGuiContext* ctx, SDL_Window* windowHandle) {
    ImGui::SetCurrentContext(ctx);

    // ================================================================
    // Setup Platform/Renderer backends
    // ================================================================

    auto graphicsContext = RHIAPI::Get()->GetGraphicsContext();
    auto swapchain       = graphicsContext->GetSwapchain();
    auto vulkanCtxWrapper =
        graphicsContext.As<VulkanGraphicsContextObj>()->GetVulkanContextWrapper();

    auto vkInstance             = vulkanCtxWrapper.Instance;
    auto vkDevice               = vulkanCtxWrapper.Device;
    auto vkPhysicalDevice       = vulkanCtxWrapper.PhysicalDevice;
    auto vkSwapchain            = vulkanCtxWrapper.Swapchain;
    auto vkQueue                = vulkanCtxWrapper.GraphicsQueue;
    auto vkGlobalDescriptorPool = vulkanCtxWrapper.GlobalDescriptorPool;

    static VkFormat colorFormats[1];
    colorFormats[0] = VulkanUtils::ToVkFormat(swapchain->GetImageFormat());

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
    init_info.ImageCount                = swapchain->GetImageCount();

    VkPipelineRenderingCreateInfoKHR dynamic_rendering_info = {};
    dynamic_rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    dynamic_rendering_info.colorAttachmentCount    = 1;
    dynamic_rendering_info.pColorAttachmentFormats = colorFormats;
    dynamic_rendering_info.depthAttachmentFormat =
        VulkanUtils::ToVkFormat(swapchain->GetDepthFormat());

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
    auto GraphicsContext = RHIAPI::Get()->GetGraphicsContext();

    auto device = GraphicsContext->GetDevice();

    device->WaitIdle();
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

    auto vkCmdBuffer = cmdList.As<VulkanCommandBufferObj>()->GetVkCommandBuffer();

    ImGui_ImplVulkan_RenderDrawData(drawData, vkCmdBuffer);
}

ImTextureID VulkanImGuiRenderer::GetTextureID(Texture texture) {
    if (!texture) return ImTextureID_Invalid;

    const UUID id = texture->GetID();

    if (auto it = m_TextureIDCache.find(id); it != m_TextureIDCache.end()) {
        if (it->second.TextureRef.Get() == texture.Get()) {
            return reinterpret_cast<ImTextureID>(it->second.DescriptorSet);
        }

        // Should not happen (ids are unique), but never hand out a registration that may point at
        // a destroyed image view.
        CZ_EDITOR_LOG(Warning,
                      "ImGui texture id {} is bound to a different texture; re-registering.",
                      id.ToString());
        ReleaseTexture(id);
    }

    auto* image = texture->GetImage().As<VulkanImageObj>();
    if (!image) {
        CZ_EDITOR_LOG(Error, "Texture '{}' has no image; drawing it is not possible.",
                      texture->GetName());
        return ImTextureID_Invalid;
    }

    VkImageView imageView = image->GetOrCreateVKView();
    if (imageView == VK_NULL_HANDLE) {
        CZ_EDITOR_LOG(Error, "Texture '{}' has no valid image view; drawing it is not possible.",
                      texture->GetName());
        return ImTextureID_Invalid;
    }

    VkDescriptorSet descSet =
        ImGui_ImplVulkan_AddTexture(imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    if (descSet == VK_NULL_HANDLE) {
        CZ_EDITOR_LOG(Error, "Failed to register texture '{}' with ImGui.", texture->GetName());
        return ImTextureID_Invalid;
    }

    m_TextureIDCache[id] = TextureEntry{ texture, descSet };

    return reinterpret_cast<ImTextureID>(descSet);
}

void VulkanImGuiRenderer::ReleaseTexture(const UUID& textureID) {
    auto it = m_TextureIDCache.find(textureID);
    if (it == m_TextureIDCache.end()) return;

    if (it->second.DescriptorSet != VK_NULL_HANDLE) {
        ImGui_ImplVulkan_RemoveTexture(it->second.DescriptorSet);
    }

    m_TextureIDCache.erase(it);
}

void VulkanImGuiRenderer::ReleaseAllTextures() {
    // Shutdown path: in-flight frames may still reference the descriptor sets we are about to
    // free.
    RHIAPI::Get()->WaitIdle();

    for (auto& [id, entry] : m_TextureIDCache) {
        if (entry.DescriptorSet != VK_NULL_HANDLE) {
            ImGui_ImplVulkan_RemoveTexture(entry.DescriptorSet);
        }
    }

    m_TextureIDCache.clear();
}