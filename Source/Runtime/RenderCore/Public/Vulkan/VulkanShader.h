#include "Core.h"
#include "Shader.h"

#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanShader, Info);

class RENDER_CORE_API CVulkanShader : public CShader {
public:
    CVulkanShader(CGraphicsContext *context, const FShaderCreateInfo &rep,
                  const FShaderCompilerOutput &vsOutput,
                  const FShaderCompilerOutput &fsOutput);
    ~CVulkanShader() {};

private:
    vk::raii::ShaderModule m_VertexModule{nullptr};
    vk::raii::ShaderModule m_FragmentModule{nullptr};
};
