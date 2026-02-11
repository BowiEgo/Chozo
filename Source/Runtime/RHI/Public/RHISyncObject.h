#pragma once

#include "RHIExport.h"
#include "Ref.h"

class RHI_API IRHISyncObject : public FRefCounted {
public:
    IRHISyncObject();
    virtual ~IRHISyncObject();
};
