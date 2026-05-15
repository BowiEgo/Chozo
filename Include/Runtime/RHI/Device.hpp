#pragma once

#include <Core/Header/Handle.hpp>
#include <Core/Memory/Memory.hpp>
#include <Runtime/RHI/CommandPool.hpp>
#include <Runtime/RHI/Sampler.hpp>

namespace CZ {

struct DeviceSpecification {
    // --- Metadata ---
    std::string AppName;
    uint32_t AppVersion;

    // --- Feature Toggles ---
    // [Note] High-level feature requests that RHI will try to fulfill
    // bool PreferIntegratedGPU = false; // Whether to use iGPU for power saving
    // bool RequireRayTracing   = false;
};

class DeviceObj;

struct Device : Handle<class DeviceObj> {
    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }

    void WaitIdle() const;

    CommandPool CreateCommandPool(CommandPoolSpecification& spec);

    Sampler GetOrCreateSampler(const SamplerSpecification spec);
};

} // namespace CZ
