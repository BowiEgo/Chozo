#pragma once

#include "RHIFrameBuffer.h"
#include "RenderCoreExport.h"
#include "Scope.h"
#include "Texture.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFrameBuffer, Info);

class RENDER_CORE_API CFrameBuffer : public FRefCounted {
public:
    CFrameBuffer(const FFrameBufferSpecification& spec);
    virtual ~CFrameBuffer() = default;

    const FFrameBufferSpecification& GetSpec() const { return m_Spec; }
    const std::string& GetName() const { return m_Spec.Name; }

    IRHIFrameBuffer* GetOrCreateResource();

    TRef<CTexture> GetColorAttachment(uint32 index) {
        auto res    = GetOrCreateResource();
        auto rhiTex = res->GetColorAttachment(index);
        if (!rhiTex) return nullptr;
        auto tex = CreateRef<CTexture>(rhiTex->GetSpec(), rhiTex.get());
        return tex;
    }

protected:
    FFrameBufferSpecification m_Spec;
    TScope<IRHIFrameBuffer> m_Resource;
};
