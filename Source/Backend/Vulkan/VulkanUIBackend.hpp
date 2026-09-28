#pragma once

#include <Runtime/UI/UIRenderBackend.hpp>

#include <vulkan/vulkan_core.h>

#include <unordered_map>

namespace CZ {

/**
 * ImGui implementation of the UI backend for the Vulkan renderer.
 *
 * This lives in the backend module because it is the only place that needs to know about ImGui's
 * Vulkan/SDL backends, native window handles and SDL events; the editor only sees
 * `UIRenderBackendObj`.
 *
 * ImGui textures are registered as descriptor sets, and those sets reference the RHI image views
 * and therefore must not outlive the texture they were created from. Registrations are keyed by
 * the texture's stable `UUID` (never reused) so that a recycled object address can never hit a
 * stale entry, and callers release them explicitly through `ReleaseTexture`.
 */
class VulkanUIBackend final : public UIRenderBackendObj {
public:
    VulkanUIBackend()           = default;
    ~VulkanUIBackend() override = default;

    bool Init(Window window, GraphicsContext ctx, std::string& err) override;
    void NewFrame() override;
    void Draw(ImDrawData* drawData, CommandList cmdList) override;

    uint64_t RegisterTexture(Texture texture) override;
    void ReleaseTexture(const UUID& textureID) override;
    void ReleaseAllTextures() override;

    void Shutdown() override;

private:
    struct TextureEntry {
        Texture TextureRef; // identity check only
        VkDescriptorSet DescriptorSet = VK_NULL_HANDLE;
    };

    // Owned by the engine; stored so the backend never has to reach for a global (the RHI's
    // facade singleton is per module, so the copy in this dylib is uninitialised).
    Window m_Window;
    GraphicsContext m_Context;

    std::unordered_map<UUID, TextureEntry> m_TextureIDCache;
};

} // namespace CZ
