#include "Shader.h"

#include "OpenGLShader.h"
#include "RendererAPI.h"
#include "VulkanContext.h"
#include "VulkanShader.h"

namespace Chozo {

Ref<Shader> Shader::Create(GraphicsContext *context,
    const ShaderCreateInfo &rep, const ShaderCompilerOutput &vsOutput,
    const ShaderCompilerOutput &fsOutput) {

    switch (RendererAPI::GetAPI()) {
    case RendererAPI::API::None:
        CZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
        return nullptr;
    case RendererAPI::API::OpenGL:
        return Ref<OpenGLShader>::Create(context, rep, vsOutput, fsOutput);
    case RendererAPI::API::Vulkan:
        return Ref<VulkanShader>::Create(context, rep, vsOutput, fsOutput);
    }

    CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}

} // namespace Chozo
