#pragma once

#include "RHIExport.h"
#include "Ref.h"

class IRHIDevice;

class RHI_API IRHIResource : public FRefCounted {
public:
    IRHIResource(const WeakRef<IRHIDevice> device);
    ~IRHIResource();

protected:
    WeakRef<IRHIDevice> m_Device;
};