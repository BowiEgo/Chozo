#pragma once

#include <Core/Header/Handle.h>
#include <Core/Memory/Memory.h>
#include <Runtime/RHI/CommandPool.h>

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

struct Device : Handle<DeviceObj> {
    static void Destroy(Device device);

    void WaitIdle() const;

    CommandPool CreateCommandPool(const CommandPoolSpecification& spec);
};

} // namespace CZ
