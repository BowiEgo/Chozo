#pragma once

#include "RHIExport.h"
#include "Ref.h"
#include "UUID.h"

class IRHIDevice;

class RHI_API IRHIResource : public FRefCounted {
public:
    IRHIResource(const WeakRef<IRHIDevice> device);
    ~IRHIResource();

    FUUID GetID() const { return m_ID; }

protected:
    WeakRef<IRHIDevice> m_Device;
    FUUID m_ID;
};