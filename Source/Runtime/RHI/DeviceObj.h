#include <Runtime/RHI/Device.h>

namespace CZ {

class DeviceObj {
public:
    DeviceObj(const DeviceSpecification& spec) : m_Spec(spec) {}
    virtual ~DeviceObj() = default;

    virtual void WaitIdle() = 0;

    virtual CommandPool CreateCommandPool(CommandPoolSpecification& spec) = 0;

protected:
    DeviceSpecification m_Spec;
};
} // namespace CZ