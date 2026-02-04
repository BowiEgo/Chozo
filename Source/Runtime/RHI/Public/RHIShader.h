#pragma once

#include "CoreMinimal.h"

class RHI_API IRHIShader : public FRefCounted {
public:
    virtual ~IRHIShader() = default;

    virtual EShaderStage GetStage() const = 0;
    virtual const std::string &GetName() const = 0;
};