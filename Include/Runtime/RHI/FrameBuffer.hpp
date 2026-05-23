#pragma once

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
public:
    FrameBufferObj(const FrameBufferSpecification& spec) : m_Spec(spec) {};

    virtual ~FrameBufferObj() { Clear(); };

    virtual void Resize(const Extent2D& size) = 0;

    Texture GetColorAttachment(uint32 index) const { return m_ColorAttachments[index]; }

protected:
    void Clear();

    FrameBufferSpecification m_Spec;

    std::vector<Texture> m_ColorAttachments;
    Texture m_DepthAttachment;
};

struct FrameBuffer : Handle<class FrameBufferObj> {
    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }
};

} // namespace CZ
