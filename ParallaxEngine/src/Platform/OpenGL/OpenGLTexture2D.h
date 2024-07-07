#pragma once

#include "Parallax/Renderer/RendererTypes.h"
#include "Parallax/Renderer/Texture.h"

namespace Parallax {
    
    class OpenGLTexture2D : public Texture2D
    {
    public:
        OpenGLTexture2D(const std::string& path);
        virtual ~OpenGLTexture2D();

        virtual uint32_t GetWidth() const override { return m_Width; };
        virtual uint32_t GetHeight() const override { return m_Height; };
		virtual RendererID GetRendererID() const override { return m_RendererID; };

        virtual void Bind(uint32_t slot = 0) const override;
    private:
        std::string m_Path;
        uint32_t m_Width, m_Height;
        RendererID m_RendererID;
    };

}