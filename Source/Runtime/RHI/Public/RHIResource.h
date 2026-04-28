#pragma once

#include "Event.h"
#include "RHIExport.h"
#include "Ref.h"
#include "UUID.h"

class IRHIDevice;

enum class EResourceType {
    Unknown,
    Texture,
    Buffer,
    Sampler,
    Image,
    SetLayout,
    DescriptorSet,
    // ...
};

class RHI_API IRHIResource : public FRefCounted {
public:
    IRHIResource(const WeakRef<IRHIDevice> device);
    ~IRHIResource();

    virtual EResourceType GetResourceType() const = 0;

    FUUID GetID() const { return m_ID; }

protected:
    WeakRef<IRHIDevice> m_Device;
    FUUID m_ID;
};

class FRHIResourceEvent : public IEvent {
public:
    IRHIResource* GetResource() const { return m_Res; }

    EVENT_CLASS_CATEGORY(EventCategory_RHIResource)
protected:
    explicit FRHIResourceEvent(IRHIResource* res) : m_Res(res) {}

    IRHIResource* m_Res;
};

class FRHIResourceDestroyedEvent : public FRHIResourceEvent {
public:
    FRHIResourceDestroyedEvent(IRHIResource* res) : FRHIResourceEvent(res) {}

    std::string ToString() const override {
        std::stringstream ss;
        ss << "RHIResourceDestroyedEvent: " << m_Res->GetID().ToString();
        return ss.str();
    }

    EVENT_CLASS_TYPE(ResourceDestroyed);
};