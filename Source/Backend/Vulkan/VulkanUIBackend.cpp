#include <cstdint>

#include "VulkanUIBackend.hpp"

#include "VulkanCommandBufferObj.hpp"
#include "VulkanGraphicsContextObj.hpp"
#include "VulkanImageObj.hpp"
#include "VulkanUtils.hpp"

#include <Core/Log/LogMacros.hpp>
#include <Runtime/Window/Window.hpp>

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

using namespace CZ;

namespace {

void CheckVKResult(VkResult err) {
    if (err == VK_SUCCESS) return;

    if (err < 0) {
        // [Note] Negative values are typically unrecoverable errors in Vulkan
        CZ_BACKEND_LOG(Fatal, "[Vulkan] Error: VkResult = {}", VulkanUtils::VkResultToString(err));
    } else {
        CZ_BACKEND_LOG(Error, "[Vulkan] Error: VkResult = {}", VulkanUtils::VkResultToString(err));
    }
}

} // namespace

bool VulkanUIBackend::Init(Window window, GraphicsContext ctx, std::string& err) {
    if (!window) {
        err = "The UI backend needs a window.";
        return false;
    }

    if (!ctx) {
        err = "The UI backend needs a graphics context.";
        return false;
    }

    m_Window  = window;
    m_Context = ctx;

    // ImGui's platform backend consumes SDL events before the engine turns them into
    // `Event`s; the window only forwards an opaque pointer, so this stays a backend detail.
    window->SetEventPreprocessor([](const void* event) {
        ImGui_ImplSDL3_ProcessEvent(static_cast<const SDL_Event*>(event));
    });

    auto* vulkanContext = ctx.As<VulkanGraphicsContextObj>();
    auto swapchain      = ctx->GetSwapchain();
    auto wrapper        = vulkanContext->GetVulkanContextWrapper();

    static VkFormat colorFormats[1];
    colorFormats[0] = VulkanUtils::ToVkFormat(swapchain->GetImageFormat());

    // `GetNativeHandle` is the platform's own handle (an NSWindow/HWND); ImGui's SDL backend
    // needs the SDL window, which the window wrapper exposes.
    ImGui_ImplSDL3_InitForVulkan(static_cast<SDL_Window*>(window->GetWindowWrapper()));

    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.ApiVersion                = VK_API_VERSION_1_4;
    initInfo.Instance                  = wrapper.Instance;
    initInfo.PhysicalDevice            = wrapper.PhysicalDevice;
    initInfo.Device                    = wrapper.Device;
    initInfo.QueueFamily               = wrapper.GraphicsQueueIndex;
    initInfo.Queue                     = wrapper.GraphicsQueue;
    initInfo.DescriptorPool            = wrapper.GlobalDescriptorPool;
    initInfo.MinImageCount             = 2;
    initInfo.ImageCount                = swapchain->GetImageCount();

    VkPipelineRenderingCreateInfoKHR dynamicRenderingInfo = {};
    dynamicRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    dynamicRenderingInfo.colorAttachmentCount    = 1;
    dynamicRenderingInfo.pColorAttachmentFormats = colorFormats;
    dynamicRenderingInfo.depthAttachmentFormat =
        VulkanUtils::ToVkFormat(swapchain->GetDepthFormat());

    initInfo.UseDynamicRendering                          = true;
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = dynamicRenderingInfo;
    initInfo.PipelineInfoMain.MSAASamples                 = VK_SAMPLE_COUNT_1_BIT;
    initInfo.CheckVkResultFn                              = CheckVKResult;

    if (!ImGui_ImplVulkan_Init(&initInfo)) {
        err = "ImGui_ImplVulkan_Init failed.";
        ImGui_ImplSDL3_Shutdown();
        return false;
    }

    return true;
}

void VulkanUIBackend::NewFrame() {
    ImGui_ImplSDL3_NewFrame();
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();
}

void VulkanUIBackend::Draw(ImDrawData* drawData, CommandList cmdList) {
    if (!drawData || drawData->TotalVtxCount == 0) return;

    VkCommandBuffer vkCmdBuffer = cmdList.As<VulkanCommandBufferObj>()->GetVkCommandBuffer();

    ImGui_ImplVulkan_RenderDrawData(drawData, vkCmdBuffer);
}

uint64_t VulkanUIBackend::RegisterTexture(Texture texture) {
    if (!texture) return 0;

    const UUID id = texture->GetID();

    if (auto it = m_TextureIDCache.find(id); it != m_TextureIDCache.end()) {
        if (it->second.TextureRef.Get() == texture.Get()) {
            return reinterpret_cast<uint64_t>(it->second.DescriptorSet);
        }

        // Should not happen (ids are unique), but never hand out a registration that may point at
        // a destroyed image view.
        CZ_BACKEND_LOG(Warning, "UI texture id {} is bound to a different texture; re-registering.",
                       id.ToString());
        ReleaseTexture(id);
    }

    auto* image = texture->GetImage().As<VulkanImageObj>();
    if (!image) {
        CZ_BACKEND_LOG(Error, "Texture '{}' has no image; it cannot be drawn by the UI.",
                       texture->GetName());
        return 0;
    }

    VkImageView imageView = image->GetOrCreateVKView();
    if (imageView == VK_NULL_HANDLE) {
        CZ_BACKEND_LOG(Error, "Texture '{}' has no valid image view; it cannot be drawn by the UI.",
                       texture->GetName());
        return 0;
    }

    VkDescriptorSet descSet =
        ImGui_ImplVulkan_AddTexture(imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    if (descSet == VK_NULL_HANDLE) {
        CZ_BACKEND_LOG(Error, "Failed to register texture '{}' with the UI.", texture->GetName());
        return 0;
    }

    m_TextureIDCache[id] = TextureEntry{ texture, descSet };

    return reinterpret_cast<uint64_t>(descSet);
}

void VulkanUIBackend::ReleaseTexture(const UUID& textureID) {
    auto it = m_TextureIDCache.find(textureID);
    if (it == m_TextureIDCache.end()) return;

    if (it->second.DescriptorSet != VK_NULL_HANDLE) {
        ImGui_ImplVulkan_RemoveTexture(it->second.DescriptorSet);
    }

    m_TextureIDCache.erase(it);
}

void VulkanUIBackend::ReleaseAllTextures() {
    // Shutdown path: in-flight frames may still reference the descriptor sets we are about to
    // free.
    if (m_Context) m_Context->GetDevice()->WaitIdle();

    for (auto& [id, entry] : m_TextureIDCache) {
        if (entry.DescriptorSet != VK_NULL_HANDLE) {
            ImGui_ImplVulkan_RemoveTexture(entry.DescriptorSet);
        }
    }

    m_TextureIDCache.clear();
}

void VulkanUIBackend::Shutdown() {
    ReleaseAllTextures();

    if (m_Context) m_Context->GetDevice()->WaitIdle();

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();

    if (m_Window) m_Window->SetEventPreprocessor(nullptr);
    m_Window  = Window();
    m_Context = GraphicsContext();
}
