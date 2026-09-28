#pragma once

#include "Core/Header/Handle.hpp"
#include <Core/Header/UUID.hpp>
#include <Runtime/RHI/CommandList.hpp>
#include <Runtime/RHI/Texture.hpp>

#include <unordered_map>

using namespace CZ;

/**
 * ImGui renderer backend owner.
 *
 * ImGui textures are registered as descriptor sets; those sets reference the RHI image views and
 * therefore must not outlive the texture they were created from. Registrations are keyed by the
 * texture's stable `UUID` (never reused) so that a recycled object address can never hit a stale
 * entry, and callers release them explicitly through `ReleaseTexture`.
 */
class VulkanImGuiRenderer {
public:
    VulkanImGuiRenderer();
    ~VulkanImGuiRenderer();

    void Init(ImGuiContext* ctx, SDL_Window* windowHandle);
    void Shutdown();
    void NewFrame();
    void Draw(ImDrawData* drawData, CommandList cmdList);

    /// Registers `texture` with ImGui, or returns the registration created earlier.
    ImTextureID GetTextureID(Texture texture);

    /// Releases the registration for a texture id. Safe to call with a dangling handle: the id is
    /// a value, nothing is dereferenced.
    void ReleaseTexture(const UUID& textureID);

    /// Releases every registration (e.g. on shutdown).
    void ReleaseAllTextures();

private:
    struct TextureEntry {
        Texture TextureRef; // identity check only
        VkDescriptorSet DescriptorSet = VK_NULL_HANDLE;
    };

    std::unordered_map<UUID, TextureEntry> m_TextureIDCache;
};