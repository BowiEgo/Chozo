#include "RHIFrameBuffer.h"

DEFINE_LOG_CATEGORY(LogRHIFrameBuffer);

IRHIFrameBuffer::IRHIFrameBuffer(const FFrameBufferSpecification& spec) : m_Spec(spec) {}

IRHIFrameBuffer::~IRHIFrameBuffer() {
    CZ_LOG(LogRHIFrameBuffer, Trace, "RHIFrameBuffer destroying...");
}
