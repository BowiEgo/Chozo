#include <Runtime/RHI/CommandPool.h>

#include "CommandPoolObj.h"

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogCommandPool, Info);

void CommandPool::Destroy(CommandPool cmdPool) {
    if (!cmdPool) return;

    Delete(cmdPool.Unwrap());
}

CommandList CommandPool::AllocateCommandBuffer() { return m_Obj->AllocateCommandBuffer(); }

} // namespace CZ