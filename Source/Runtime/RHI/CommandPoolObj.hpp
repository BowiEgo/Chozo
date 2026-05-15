#include <Runtime/RHI/CommandPool.hpp>
#include <Runtime/RHI/Device.hpp>

namespace CZ {

class CommandPoolObj {
public:
    CommandPoolObj(CommandPoolSpecification& spec) : m_Spec(spec) {}
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
