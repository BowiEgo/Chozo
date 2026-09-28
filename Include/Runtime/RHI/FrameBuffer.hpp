#pragma once
#include <string>
#include <vector>

#include <Core/Header/Extent.hpp>
#include <Core/Header/Handle.hpp>
#include <Core/Header/Types.h>
#include <Core/Memory/Memory.hpp>
#include <Runtime/RHI/RHITypes.hpp>
#include <Runtime/RHI/Texture.hpp>

namespace CZ {

struct FrameBufferSpecification {
    std::string Name;
    Extent2D Size;
    std::vector<PixelFormat> ColorFormats;
    PixelFormat DepthFormat = PixelFormat::Unknown;
};

class FrameBufferObj {
    friend class Handle<FrameBufferObj>;

public:
    explicit FrameBufferObj(const FrameBufferSpecification& spec) : m_Spec(spec) {};

    virtual ~FrameBufferObj() { Clear(); };

    virtual void Resize(const Extent2D& size) = 0;

    Texture GetColorAttachment(uint32 index) const {
        return index < m_ColorAttachments.size() ? ViewAs<Texture>(m_ColorAttachments[index])
                                                 : Texture();
    }

    uint32 GetColorAttachmentCount() const {
        return static_cast<uint32>(m_ColorAttachments.size());
    }

    Texture GetDepthAttachment() const { return ViewAs<Texture>(m_DepthAttachment); }

protected:
    void Clear() {
        m_ColorAttachments.clear();
        m_DepthAttachment.reset();
    }

    FrameBufferSpecification m_Spec;

    // Attachments are owned by the framebuffer and handed out as views.
    std::vector<Scope<TextureObj>> m_ColorAttachments;
    Scope<TextureObj> m_DepthAttachment;
};

struct FrameBuffer : Handle<class FrameBufferObj> {
    using Handle<class FrameBufferObj>::Handle;

    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }
};

} // namespace CZ
