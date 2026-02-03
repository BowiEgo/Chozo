#include "Core.h"
#include "Shader.h"

#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>

namespace Chozo {

DECLARE_LOG_CATEGORY_EXTERN(LogVulkanShader, Info);

class RENDERCORE_API VulkanShader : public Shader {
public:
    VulkanShader(GraphicsContext *context, const ShaderCreateInfo &rep,
                 const ShaderCompilerOutput &vsOutput,
                 const ShaderCompilerOutput &fsOutput);
    ~VulkanShader() {};

    void Bind() const override;
    void Unbind() const override;
    void ClearCache() override;
    void Compile() override;
    void AsyncCompile() override;

private:
    vk::raii::ShaderModule m_VertexModule{nullptr};
    vk::raii::ShaderModule m_FragmentModule{nullptr};
};
} // namespace Chozo
