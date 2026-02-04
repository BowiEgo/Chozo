#include "Shader.h"

#include "OpenGLShader.h"
#include "RendererAPI.h"
#include "VulkanContext.h"
#include "VulkanShader.h"

TRef<CShader> CShader::Create(CGraphicsContext *context,
                              const FShaderCreateInfo &rep,
                              const FShaderCompilerOutput &vsOutput,
                              const FShaderCompilerOutput &fsOutput) {

    switch (CRendererAPI::GetType()) {
    case CRendererAPI::EType::None:
        CZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
        return nullptr;
    case CRendererAPI::EType::OpenGL:
        return TRef<COpenGLShader>::Create(context, rep, vsOutput, fsOutput);
    case CRendererAPI::EType::Vulkan:
        return TRef<CVulkanShader>::Create(context, rep, vsOutput, fsOutput);
    }

    CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
