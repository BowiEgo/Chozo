#include <Runtime/RHI/CommandPool.h>
#include <Runtime/RHI/Device.h>

namespace CZ {

class CommandPoolObj {
public:
    CommandPoolObj(const CommandPoolSpecification& spec) : m_Spec(spec) {}
    virtual ~CommandPoolObj() = default;

    CommandPoolObj(const CommandPoolObj&)            = delete;
    CommandPoolObj& operator=(const CommandPoolObj&) = delete;
    CommandPoolObj(CommandPoolObj&&)                 = delete;
    CommandPoolObj& operator=(CommandPoolObj&&)      = delete;

    virtual CommandList AllocateCommandBuffer() = 0;

protected:
    CommandPoolSpecification m_Spec;
};
} // namespace CZ
