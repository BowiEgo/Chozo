#pragma once

#include "CoreMinimal.h"
#include "GraphicsContext.h"
#include "RHITypes.h"

class RENDERCORE_API CShader : public FRefCounted {
protected:
    CShader(CGraphicsContext *context, const FShaderCreateInfo &rep)
        : m_Context(context), m_Rep(rep) {};

public:
    virtual ~CShader() = default;

    const std::string &GetName() const { return m_Rep.Name; }
    const FShaderID &GetID() const { return m_ID; }

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;
    virtual void ClearCache() = 0;
    virtual void Compile() = 0;
    virtual void AsyncCompile() = 0;

    static TRef<CShader> Create(CGraphicsContext *context,
                                const FShaderCreateInfo &rep,
                                const FShaderCompilerOutput &vsOutput,
                                const FShaderCompilerOutput &fsOutput);

protected:
    FShaderID m_ID{};
    FShaderCreateInfo m_Rep;
    CGraphicsContext *m_Context;
    FShaderReflection m_Reflection;
};
