#pragma once

#include <prxpch.h>

namespace Parallax {
    
    class Shader
    {
    private:
        uint32_t m_RendererID;
    public:
        Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
        ~Shader();

        void Bind() const;
        void Unbind() const;
    };
}