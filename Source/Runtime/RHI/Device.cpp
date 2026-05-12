#include "DeviceObj.h"

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogDevice, Info);

void Device::Destroy(Device device) {
    if (!device) return;

    Delete(device.Unwrap());
}

void Device::WaitIdle() const { m_Obj->WaitIdle(); }

CommandPool Device::CreateCommandPool(const CommandPoolSpecification& spec) {
    return m_Obj->CreateCommandPool(spec);
};

} // namespace CZ