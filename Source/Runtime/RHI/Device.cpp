#include <Runtime/RHI/Device.h>

#include <Core/DynamicLibrary/BackendRegistry.h>
#include <Core/Log/LogMacros.h>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogDevice, Info);

void Device::Destroy(Device device) {
    if (!device) return;

    auto& registry = BackendRegistry::Get();
    auto destroyFn = registry.GetFunction<void (*)(DeviceObj*)>("vulkan", "DestroyVulkanDeviceObj");
    if (destroyFn) {
        destroyFn(static_cast<DeviceObj*>(device.Unwrap()));
    } else {
        CZ_LOG(LogDevice, Error, "DestroyVulkanDeviceObj not found, memory leak possible.");
    }
}

} // namespace CZ