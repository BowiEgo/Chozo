#include <Runtime/RHI/Device.hpp>

namespace CZ {

class DeviceObj {
public:
    DeviceObj(const DeviceSpecification& spec) : m_Spec(spec) {}
    virtual ~DeviceObj() = default;

    virtual void WaitIdle() = 0;

    virtual CommandPool CreateCommandPool(CommandPoolSpecification& spec) = 0;

    virtual Sampler CreateSampler(const SamplerSpecification spec) = 0;

    Sampler GetOrCreateSampler(const SamplerSpecification spec) {
        auto it = m_SamplerCache.find(spec);
        if (it != m_SamplerCache.end()) {
            return it->second;
        }

        Sampler sampler      = CreateSampler(spec);
        m_SamplerCache[spec] = sampler;

        return sampler;
    }

protected:
    DeviceSpecification m_Spec;

    std::unordered_map<SamplerSpecification, Sampler> m_SamplerCache;
};
} // namespace CZ