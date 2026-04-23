#pragma once

#include "CoreMinimal.h"
#include "ImGuiRenderer.h"
#include "VulkanContext.h"
#include "VulkanImGuiExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanImGuiRenderer, Info);

class VULKAN_IM_GUI_API CVulkanImGuiRenderer : public IImGuiRenderer {
public:
    CVulkanImGuiRenderer(CWindow* window, IRHIContext* rhiContext);
    virtual ~CVulkanImGuiRenderer() override = default;

    virtual void Init(ImGuiContext* ctx) override;
    virtual void Shutdown() override;
    virtual void NewFrame() override;
    virtual void Draw(ImDrawData* drawData, const TRef<IRHICommandList>& cmdBuffer) override;

    virtual ImTextureID GetTextureIDForRHITexture(const IRHITexture* texture) override;
    virtual void ReleaseTextureID(ImTextureID id) override;
};