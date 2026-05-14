#pragma once

#include <Runtime/RHI/CommandList.h>
#include <Runtime/RHI/Texture.h>

using namespace CZ;

class VulkanImGuiRenderer {
public:
    VulkanImGuiRenderer();
    ~VulkanImGuiRenderer();

    void Init(ImGuiContext* ctx, SDL_Window* windowHandle);
    void Shutdown();
    void NewFrame();
    void Draw(ImDrawData* drawData, CommandList cmdList);

    ImTextureID GetTextureIDForRHITexture(Texture texture);

private:
    std::unordered_map<const Texture, ImTextureID> m_TextureIDCache;
};