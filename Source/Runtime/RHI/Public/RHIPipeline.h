#pragma once

#include "RHIExport.h"
#include "Ref.h"

struct FRHIPipelineCreateInfo {
    std::string Name;
};

class RHI_API IRHIPipeline : public FRefCounted {
public:
    IRHIPipeline(const FRHIPipelineCreateInfo& info);
    virtual ~IRHIPipeline();

    virtual void Bind() = 0;

protected:
    std::string m_Name;
};