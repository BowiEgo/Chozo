#pragma once

#include "GraphicsContext.h"
#include "RHIShader.h"
#include "Ref.h"
#include "RenderCoreExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogShader, Info);

class RENDER_CORE_API CShader : public FRefCounted {
public:
    CShader(const TRef<IRHIDevice> device, const FShaderCompiledData& data);
    virtual ~CShader() = default;

    const std::string& GetName() const { return m_Name; }
    const FShaderID& GetID() const { return m_ID; }

    void CreateRHIDeviceResource(const FShaderCompiledData& data);

protected:
    FShaderID m_ID{};
    std::string m_Name;
    TRef<IRHIDevice> m_Device;
    // FShaderCreateInfo m_Rep;
    // CGraphicsContext *m_Context;
    FShaderReflection m_Reflection;
    TRef<IRHIShader> m_RHIShaders[kShaderStageCount];
};
