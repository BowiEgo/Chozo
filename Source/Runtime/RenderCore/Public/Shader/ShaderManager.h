#pragma once

#include "Core.h"
#include "GraphicsContext.h"
#include "RenderCoreExport.h"
#include "Shader.h"
#include "ShaderCompiler.h"

DECLARE_LOG_CATEGORY_EXTERN(LogShaderManager, Info);

class RENDER_CORE_API CShaderManager : public FRefCounted {
public:
    CShaderManager() = default;
    ~CShaderManager() = default;

    static void Init();
    static const CShaderManager* Get() {
        CZ_CORE_ASSERT(
            s_Instance,
            "ShaderManager instance is null! Did you forget to call Init()?");
        return s_Instance;
    }
    // static const CGraphicsContext *GetGraphicsContext() { return m_Context; }

    TRef<CShader> Load(const FShaderCreateInfo& rep);

private:
    static CShaderManager* s_Instance;
    // CGraphicsContext *m_Context;

    std::unordered_map<FShaderID, TRef<CShader>> m_ShaderCache;
    TScope<CShaderCompiler> m_Compiler;
};
