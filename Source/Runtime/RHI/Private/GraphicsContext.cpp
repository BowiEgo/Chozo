#include "GraphicsContext.h"

DEFINE_LOG_CATEGORY(LogGraphicsContext);

IGraphicsContext::IGraphicsContext(const FRHIWindowInfo& windowInfo) : m_WindowInfo(windowInfo) {}

IGraphicsContext::~IGraphicsContext() {
    CZ_LOG(LogGraphicsContext, Trace, "GraphicsContext destroying...");
}
