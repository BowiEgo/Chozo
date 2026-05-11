#include <Runtime/RHI/GraphicContext.h>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogGraphicContext, Info);

void GraphicContext::Destroy(GraphicContext ctx) {
    if (!ctx) return;

    Delete(ctx.Unwrap());
}

} // namespace CZ