#pragma once

#include "CoreMinimal.h"
#include "RHICommandList.h"
#include "RHIContext.h"
#include "RHIFrameBuffer.h"
#include "Scene.h"
#include "SceneCamera.h"
#include "Scope.h"

#include "RenderCoreExport.h"

struct FViewportSpecification {
    std::string Name;
    uint32 Width = 1, Height = 1;
};

class RENDER_CORE_API CViewport {
public:
    CViewport(IRHIContext* ctx, const FViewportSpecification& spec);
    ~CViewport() = default;

    void BeginRender(IRHICommandList* cmdList, TRef<IRHIPipeline> pipeline);
    void EndRender(IRHICommandList* cmdList);
    void Resize(uint32_t width, uint32_t height);

    void SetScene(FScene* scene) { m_Scene = scene; }

    FScene* GetScene() const { return m_Scene; }
    TRef<CSceneCamera> GetCamera() { return m_Camera; }
    const TRef<CSceneCamera> GetCamera() const { return m_Camera; }
    TRef<IRHIFrameBuffer> GetFrameBuffer() const { return m_FrameBuffer; }
    void* GetTextureID(int index) const {
        return m_FrameBuffer->GetColorAttachment(index)->GetDescriptorSet();
    }

    const std::string& GetName() const { return m_Spec.Name; }
    uint32_t GetWidth() const { return m_Spec.Width; }
    uint32_t GetHeight() const { return m_Spec.Height; }
    float GetAspectRatio() const { return (float)m_Spec.Width / m_Spec.Height; }

private:
    void CreateFrameBuffer();

private:
    IRHIContext* m_Context;
    FViewportSpecification m_Spec;

    FScene* m_Scene;
    TRef<CSceneCamera> m_Camera;
    TRef<IRHIFrameBuffer> m_FrameBuffer;

    // FMatrix4 m_ViewportMatrix;
};