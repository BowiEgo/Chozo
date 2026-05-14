#include <Runtime/RHI/Fence.h>

#include "FenceObj.h"

namespace CZ {

DEFINE_HANDLE_DESTROY(FenceObj)

bool Fence::WaitAndReset(uint64_t timeout) const { return m_Obj->WaitAndReset(timeout); }

} // namespace CZ