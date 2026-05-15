#include "DeviceObj.hpp"

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogDevice, Info);

DEFINE_HANDLE_DESTROY(DeviceObj)

void Device::WaitIdle() const { m_Obj->WaitIdle(); }

CommandPool Device::CreateCommandPool(CommandPoolSpecification& spec) {
    return m_Obj->CreateCommandPool(spec);
};

Sampler Device::GetOrCreateSampler(const SamplerSpecification spec) {
    return m_Obj->GetOrCreateSampler(spec);
}

} // namespace CZ