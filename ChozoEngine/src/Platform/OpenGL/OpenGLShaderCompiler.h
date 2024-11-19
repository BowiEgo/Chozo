#pragma once

#include "Chozo/Renderer/Shader/ShaderCompiler.h"
#include <glad/glad.h>

namespace Chozo {

    class OpenGLShaderCompiler : public ShaderCompiler
    {
    public:
        OpenGLShaderCompiler() = default;

        virtual RendererID Compile(const std::vector<std::string> filePaths) override;
        virtual void Release() override;
    private:
        void DecompileVulkanBinaries();
        RendererID CompileToProgram();
    private:
        ShaderBinaries m_OpenGLSPIRV;
        ShaderSources m_OpenGLSourceCode;
    };
} // namespace Chozo
