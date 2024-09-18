#pragma once

#include "Chozo/Renderer/RendererTypes.h"
#include "Chozo/Renderer/Texture.h"

#include <glad/glad.h>

namespace Chozo {
    
    class OpenGLTexture2D : public Texture2D
    {
    public:
        OpenGLTexture2D(const TextureSpecification& spec);
        OpenGLTexture2D(const std::string& path, const TextureSpecification& spec);
        OpenGLTexture2D(const RendererID& id, const TextureSpecification& spec);
        virtual ~OpenGLTexture2D();

        virtual uint32_t GetWidth() const override { return m_Width; };
        virtual uint32_t GetHeight() const override { return m_Height; };
		virtual RendererID GetRendererID() const override { return m_RendererID; };
        virtual TextureSpecification GetSpecification() const override { return m_Spec; };

        virtual void Resize(uint32_t width, uint32_t height) override;

        virtual void Bind(uint32_t slot = 0) const override;
        virtual void Unbind() const override;

        virtual void SetData(const void* data, const uint32_t size) override;
    private:
        TextureSpecification m_Spec;
        std::string m_Path;
        uint32_t m_Width, m_Height;
        RendererID m_RendererID;
        GLenum m_InternalFormat = GL_RGBA8, m_DataFormat = GL_RGBA, m_DataType = GL_UNSIGNED_BYTE;
        unsigned char* m_DataBuffer = nullptr;
    };

    class OpenGLTextureCube : public TextureCube
    {
    public:
        OpenGLTextureCube(const TextureCubeSpecification& spec);
        virtual ~OpenGLTextureCube();

        void Invalidate();

        virtual uint32_t GetWidth() const override { return m_Width; };
        virtual uint32_t GetHeight() const override { return m_Height; };
		virtual RendererID GetRendererID() const override { return m_RendererID; };

        virtual void Bind(uint32_t slot = 0) const override;
        virtual void Unbind() const override;

        virtual void SetData(void* data, uint32_t size) override;
    private:
        TextureCubeSpecification m_Spec;
        uint32_t m_Width, m_Height;
        RendererID m_RendererID;
        void* m_LocalBuffer;
    };
}