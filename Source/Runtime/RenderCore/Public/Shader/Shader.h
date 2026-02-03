#pragma once

#include "CoreMinimal.h"
#include "GraphicsContext.h"
#include "ShaderTypes.h"

namespace Chozo {

class RENDERCORE_API Shader : public RefCounted {
protected:
    Shader(GraphicsContext *context, const ShaderCreateInfo &rep)
        : m_Context(context), m_Rep(rep) {};

public:
    virtual ~Shader() = default;

    const std::string &GetName() const { return m_Rep.Name; }
    const ShaderID &GetID() const { return m_ID; }

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;
    virtual void ClearCache() = 0;
    virtual void Compile() = 0;
    virtual void AsyncCompile() = 0;

    static Ref<Shader> Create(GraphicsContext *context,
        const ShaderCreateInfo &rep, const ShaderCompilerOutput &vsOutput,
        const ShaderCompilerOutput &fsOutput);

protected:
    ShaderID m_ID{};
    ShaderCreateInfo m_Rep;
    GraphicsContext *m_Context;
    ShaderReflection m_Reflection;
};

} // namespace Chozo
