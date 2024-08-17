#pragma once

#include "Chozo/Renderer/RendererTypes.h"
#include "Chozo/Renderer/Texture.h"

namespace Chozo {
    
    class OpenGLTexture2D : public Texture2D
    {
    public:
        OpenGLTexture2D(const TextureSpecification& spec);
        OpenGLTexture2D(const std::string& path, const TextureSpecification& spec);
        virtual ~OpenGLTexture2D();

        virtual uint32_t GetWidth() const override { return m_Width; };
        virtual uint32_t GetHeight() const override { return m_Height; };
		virtual RendererID GetRendererID() const override { return m_RendererID; };

        virtual void Bind(uint32_t slot = 0) const override;
        virtual void Unbind() const override;

        virtual void SetData(const void* data, const uint32_t size) override;
    private:
        TextureSpecification m_Spec;
        std::string m_Path;
        uint32_t m_Width, m_Height;
        RendererID m_RendererID;
    };

    class OpenGLTextureCube : public TextureCube
    {
    public:
        OpenGLTextureCube(const TextureSpecification& spec);
        virtual ~OpenGLTextureCube();

        void Invalidate();

        virtual uint32_t GetWidth() const override { return m_Width; };
        virtual uint32_t GetHeight() const override { return m_Height; };
		virtual RendererID GetRendererID() const override { return m_RendererID; };

        virtual void Bind(uint32_t slot = 0) const override;
        virtual void Unbind() const override;
    private:
        TextureSpecification m_Spec;
        uint32_t m_Width, m_Height;
        RendererID m_RendererID;
    };
}