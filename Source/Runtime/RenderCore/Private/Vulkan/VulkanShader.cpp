#include "VulkanShader.h"
#include "Logger.h"
#include "ShaderCompiler.h"
#include "VulkanContext.h"

namespace Chozo {

DEFINE_LOG_CATEGORY(LogVulkanShader);

VulkanShader::VulkanShader(GraphicsContext *context,
                           const ShaderCreateInfo &rep,
                           const ShaderCompilerOutput &vsOutput,
                           const ShaderCompilerOutput &fsOutput)
    : Shader(context, rep) {

    auto *vkContext = dynamic_cast<VulkanContext *>(m_Context);
    auto &device = vkContext->GetDevice();

    try {
        m_VertexModule = vk::raii::ShaderModule(
            device, vk::ShaderModuleCreateInfo(
                        {}, vsOutput.Binary.size() * sizeof(uint32_t),
                        vsOutput.Binary.data()));

        m_FragmentModule = vk::raii::ShaderModule(
            device, vk::ShaderModuleCreateInfo(
                        {}, fsOutput.Binary.size() * sizeof(uint32_t),
                        fsOutput.Binary.data()));

        CZ_LOG(LogVulkanShader, Info, "Vulkan Shader Module created: {0}",
               rep.Name);
    } catch (const std::exception &e) {
        CZ_LOG(LogVulkanShader, Error,
               "Failed to create Vulkan shader module: {0}", e.what());
    }

    // Store reflection for Pipeline Layout creation
    m_Reflection = vsOutput.Reflection;
    // m_Reflection.Merge(fsOutput.Reflection); // Combine VS and FS bindings
}

void VulkanShader::Bind() const {}
void VulkanShader::Unbind() const {}
void VulkanShader::ClearCache() {}
void VulkanShader::Compile() {
    CZ_LOG(LogVulkanShader, Trace, "Compiling shader '{}'", m_Rep.Name);
}
void VulkanShader::AsyncCompile() {}
} // namespace Chozo
