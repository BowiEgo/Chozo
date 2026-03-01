#pragma once

#include "Core.h"
#include "RHIDevice.h"
#include "RenderCoreExport.h"
#include "Shader.h"
#include "ShaderCompiler.h"

DECLARE_LOG_CATEGORY_EXTERN(LogShaderManager, Info);

class RENDER_CORE_API CShaderManager : public FRefCounted {
public:
    CShaderManager() = default;
    ~CShaderManager() = default;

    static void Init(const TRef<IRHIDevice>& device);
    static CShaderManager* Get() {
        CZ_CORE_ASSERT(s_Instance,
                       "ShaderManager instance is null! Did you forget to call Init()?");
        return s_Instance;
    }
    // static const CGraphicsContext *GetGraphicContext() { return m_Context; }

    TRef<CShader> Load(const FShaderSpecification& spec);

private:
    static CShaderManager* s_Instance;
    // CGraphicsContext *m_Context;

    // std::unordered_map<FShaderID, TRef<CShader>> m_ShaderCache;
    std::unordered_map<std::string, std::string> m_ShaderSourceCache;
    TScope<CShaderCompiler> m_Compiler;
    WeakRef<IRHIDevice> m_Device;
};
