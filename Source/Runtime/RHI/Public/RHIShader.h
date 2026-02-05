#pragma once

#include "RHITypes.h"
#include "Ref.h"

class RHI_API IRHIShader : public FRefCounted {
public:
    IRHIShader();
    virtual ~IRHIShader();

    virtual EShaderStage GetStage() const = 0;
    virtual const std::string &GetName() const = 0;
};