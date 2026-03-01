#pragma once

#include "RHIExport.h"
#include "RHITexture2D.h"
#include "RHITypes.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHIFrameBuffer, Info);

struct FFrameBufferSpecification {
    std::string Name;
    FExtent2D Size;
    std::vector<EPixelFormat> ColorFormats;
    EPixelFormat DepthFormat = EPixelFormat::Unknown;
};

class RHI_API IRHIFrameBuffer : public FRefCounted {
public:
    IRHIFrameBuffer(const FFrameBufferSpecification& spec);
    virtual ~IRHIFrameBuffer();

    virtual void Resize(uint32 width, uint32 height) = 0;

    TRef<IRHITexture2D> GetColorAttachment(uint32 index) const { return m_ColorAttachments[index]; }

protected:
    FFrameBufferSpecification m_Spec;

    std::vector<TRef<IRHITexture2D>> m_ColorAttachments;
    TRef<IRHITexture2D> m_DepthAttachment;
};