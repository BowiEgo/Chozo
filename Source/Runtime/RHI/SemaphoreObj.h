#include <Runtime/RHI/Semaphore.h>

namespace CZ {

class SemaphoreObj {
public:
    SemaphoreObj() = default;

    virtual ~SemaphoreObj() { CZ_CORE_LOG(Error, "SemaphoreObj destructed"); }
};
} // namespace CZ