#include <Runtime/RHI/CommandPool.hpp>

#include "CommandPoolObj.hpp"

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogCommandPool, Info);

DEFINE_HANDLE_DESTROY(CommandPoolObj)

CommandList CommandPool::AllocateCommandBuffer() { return m_Obj->AllocateCommandBuffer(); }

} // namespace CZ