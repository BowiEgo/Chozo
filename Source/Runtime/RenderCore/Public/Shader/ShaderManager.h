#pragma once

#include "Core.h"
#include "GraphicsContext.h"
#include "Shader.h"
#include "ShaderCompiler.h"

namespace Chozo {

DECLARE_LOG_CATEGORY_EXTERN(LogShaderManager, Info);

class RENDERCORE_API ShaderManager : public RefCounted {
public:
    ShaderManager() = default;
    ~ShaderManager() = default;

    static void Init(GraphicsContext *context);
    static ShaderManager *Get() {
        CZ_CORE_ASSERT(
            s_Instance,
            "ShaderManager instance is null! Did you forget to call Init()?");
        return s_Instance;
    }
    Ref<Shader> Load(const ShaderCreateInfo &rep);

private:
    static ShaderManager *s_Instance;
    GraphicsContext *m_Context;

    std::unordered_map<ShaderID, Ref<Shader>> m_ShaderCache;
    Scope<ShaderCompiler> m_Compiler;
};
} // namespace Chozo
