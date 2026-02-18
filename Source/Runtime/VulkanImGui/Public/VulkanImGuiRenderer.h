#pragma once

#include "CoreMinimal.h"
#include "ImGuiRenderer.h"
#include "VulkanImGuiExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanImGuiRenderer, Info);

class VULKAN_IM_GUI_API CVulkanImGuiRenderer : public IImGuiRenderer {
public:
    CVulkanImGuiRenderer(CWindow* window, CGraphicsContext* context);
    virtual ~CVulkanImGuiRenderer() override = default;

    virtual void Init(ImGuiContext* ctx) override;
    virtual void Shutdown() override;
    virtual void NewFrame() override;
    virtual void Render(ImDrawData* drawData, const TRef<IRHICommandBuffer> cmdBuffer) override;
};