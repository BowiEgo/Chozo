#pragma once

#include "CoreMinimal.h"
#include "RHICommandList.h"
#include "RHIContext.h"
#include "UIExport.h"
#include "Window.h"

#include <imgui_internal.h>

DECLARE_LOG_CATEGORY_EXTERN(LogImGuiRenderer, Info);

class UI_API IImGuiRenderer {
public:
    IImGuiRenderer(CWindow* window, IRHIContext* rhiContext);
    virtual ~IImGuiRenderer();

    virtual void Init(ImGuiContext* ctx)                                          = 0;
    virtual void Shutdown()                                                       = 0;
    virtual void NewFrame()                                                       = 0;
    virtual void Draw(ImDrawData* drawData, const TRef<IRHICommandList>& cmdList) = 0;

    virtual ImTextureID GetTextureIDForRHITexture(const IRHITexture* texture) = 0;
    virtual void ReleaseTextureID(ImTextureID id)                             = 0;

protected:
    CWindow* m_Window;
    IRHIContext* m_Context;
    bool m_bShutdown = false;

    TScope<IRHITexture> m_DefaultBlackTexture;
    std::unordered_map<const IRHITexture*, ImTextureID> m_TextureIDCache;
};