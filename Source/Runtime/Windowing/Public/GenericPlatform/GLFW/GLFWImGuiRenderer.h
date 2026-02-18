#pragma once

#include "ImGuiRenderer.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCGLFWImGuiRenderer, Info);

class CVulkanRHICommandBuffer;

class WINDOWING_API CGLFWImGuiRenderer : public IImGuiRenderer {
public:
    CGLFWImGuiRenderer();
    virtual ~CGLFWImGuiRenderer() override = default;

    virtual void Init() override;
    virtual void Shutdown() override;
    virtual void NewFrame() override;
    virtual void Render(ImDrawData* drawData, const TRef<IRHICommandBuffer> cmdBuffer) override;
}