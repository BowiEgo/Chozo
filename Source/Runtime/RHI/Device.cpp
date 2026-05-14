#include "DeviceObj.h"

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogDevice, Info);

DEFINE_HANDLE_DESTROY(DeviceObj)

// template <> void Handle<DeviceObj>::Destroy() {
//     auto& registry = DynamicLibraryRegistry::Get();

//     auto destroyFn =
//         registry.GetFunction<void (*)(DeviceObj* obj)>("vulkan_backend",
//         "DestroyVulkanDeviceObj");
//     if (!destroyFn) {
//         CZ_LOG(LogGraphicsContext, Error, "DestroyVulkanDeviceObj not found in backend.");
//     }

//     if (m_Obj) {
//         destroyFn(m_Obj);
//         m_Obj = nullptr;
//     }
// }

void Device::WaitIdle() const { m_Obj->WaitIdle(); }

CommandPool Device::CreateCommandPool(CommandPoolSpecification& spec) {
    return m_Obj->CreateCommandPool(spec);
};

} // namespace CZ