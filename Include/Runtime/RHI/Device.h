#pragma once

#include <Core/Header/Handle.h>
#include <Core/Memory/Memory.h>
#include <Runtime/RHI/GraphicContext.h>

namespace CZ {

struct DeviceSpecification {
    GraphicContext GraphicContext;

    // --- Metadata ---
    std::string AppName;
    uint32_t AppVersion;

    // --- Feature Toggles ---
    // [Note] High-level feature requests that RHI will try to fulfill
    // bool PreferIntegratedGPU = false; // Whether to use iGPU for power saving
    // bool RequireRayTracing   = false;
};

class DeviceObj {
public:
    DeviceObj(const DeviceSpecification& spec) : m_Spec(spec) {}
    virtual ~DeviceObj() = default;

    virtual void WaitIdle() = 0;

    GraphicContext GetGraphicContext() const { return m_Spec.GraphicContext; }

protected:
    DeviceSpecification m_Spec;
};

struct Device : Handle<DeviceObj> {
    static void Destroy(Device device);

    void WaitIdle() const { m_Obj->WaitIdle(); }
};

} // namespace CZ
