#include "FrameBuffer.h"

#include "RHIAPI.h"

DEFINE_LOG_CATEGORY(LogFrameBuffer);

CFrameBuffer::CFrameBuffer(const FFrameBufferSpecification& spec) : m_Spec(spec) {
    CZ_LOG(LogFrameBuffer, Trace, "Creating FrameBuffer {} ...", m_Spec.Name);
}

IRHIFrameBuffer* CFrameBuffer::GetResource() {
    if (m_Resource) {
        return m_Resource.get();
    }

    m_Resource = TScope<IRHIFrameBuffer>(IRHIAPI::CreateFrameBuffer(m_Spec).get());

    CZ_LOG(LogFrameBuffer, Info, "RHI FrameBuffer: {} created.", m_Spec.Name);

    return m_Resource.get();
}
