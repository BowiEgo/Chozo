#pragma once

#include <Runtime/RHI/CommandList.h>

using namespace CZ;

class VulkanImGuiRenderer {
public:
    VulkanImGuiRenderer();
    ~VulkanImGuiRenderer();

    void Init(ImGuiContext* ctx, SDL_Window* windowHandle);
    void Shutdown();
    void NewFrame();
    void Draw(ImDrawData* drawData, CommandList cmdList);
};