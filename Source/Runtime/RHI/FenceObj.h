#include <Runtime/RHI/Fence.h>

namespace CZ {

class FenceObj {
public:
    FenceObj() {};

    virtual ~FenceObj() {};

    virtual bool WaitAndReset(uint64_t timeout) const = 0;
};
} // namespace CZ