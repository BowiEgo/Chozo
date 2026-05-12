#include <Runtime/RHI/GraphicsContext.h>

#include "GraphicsContextObj.h"

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogGraphicsContext, Info);

void GraphicsContext::Destroy(GraphicsContext ctx) {
    if (!ctx) return;

    Delete(ctx.Unwrap());
}

Device GraphicsContext::GetDevice() { return m_Obj->GetDevice(); }

Swapchain GraphicsContext::GetSwapchain() { return m_Obj->GetSwapchain(); }

} // namespace CZ