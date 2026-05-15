#pragma once

#include "Core/Header/Handle.hpp"
#include <Runtime/RHI/CommandList.hpp>
#include <Runtime/RHI/Texture.hpp>

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
    HandleMap<Texture> m_TextureIDCache;
};