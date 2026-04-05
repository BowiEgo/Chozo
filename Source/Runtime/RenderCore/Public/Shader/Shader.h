#pragma once

#include "RHIContext.h"
#include "RHIShader.h"
#include "Ref.h"
#include "RenderCoreExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogShader, Info);

class RENDER_CORE_API CShader : public FRefCounted {
public:
    CShader(const FShaderSpecification& spec, const FShaderCompilerOutput& data,
            const WeakRef<IRHIDevice> device);
    virtual ~CShader() = default;

    const FShaderID& GetID() const { return m_ID; }
    const std::string& GetName() const { return m_Spec.Name; }
    TRef<IRHIShader> GetShaderResource(const IRHIContext* ctx) {
        if (!m_ShaderResource) {
            m_ShaderResource = CreateRHIDeviceResource(ctx, m_Data);
        }
        return m_ShaderResource;
    }

    TRef<IRHIShader> CreateRHIDeviceResource(const IRHIContext* ctx,
                                             const FShaderCompilerOutput& data);

protected:
    FShaderSpecification m_Spec;

    FShaderID m_ID{};
    FShaderCompilerOutput m_Data;
    WeakRef<IRHIDevice> m_Device;
    TRef<IRHIShader> m_ShaderResource;
};
