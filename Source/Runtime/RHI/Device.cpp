#include <Runtime/RHI/Device.h>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogDevice, Info);

void Device::Destroy(Device device) {
    if (!device) return;

    Delete(device.Unwrap());
}

} // namespace CZ