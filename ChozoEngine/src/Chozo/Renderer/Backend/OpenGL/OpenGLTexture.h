#pragma once

#include "Chozo/Renderer/RendererTypes.h"
#include "Chozo/Renderer/Texture.h"

#include <glad/glad.h>

namespace Chozo {

    class OpenGLTexture2D : public Texture2D
    {
    public:
        OpenGLTexture2D(const Texture2DSpecification& spec);
        OpenGLTexture2D(Ref<OpenGLTexture2D> other);
        OpenGLTexture2D(const std::string& path, const Texture2DSpecification& spec);
        OpenGLTexture2D(const RendererID& id, const Texture2DSpecification& spec);
        OpenGLTexture2D(const Buffer& imageBuffer, const Texture2DSpecification& spec);

        virtual ~OpenGLTexture2D();

        uint32_t GetWidth() const override { return m_Width; };
        uint32_t GetHeight() const override { return m_Height; };
		RendererID GetRendererID() const override { return m_RendererID; };
        Texture2DSpecification GetSpecification() const override { return m_Spec; };

        void Resize(uint32_t width, uint32_t height) override;

        void SetData(const void* data, uint32_t size) override;
        void ExtractBuffer() override;
        void CopyToHostBuffer(Buffer& buffer) const override;

        void Bind(uint32_t slot = 0) const;
        void Unbind() const;
    private:
        void Invalidate();
    private:
        Texture2DSpecification m_Spec;
        std::string m_Path;
        uint32_t m_Width, m_Height;
        RendererID m_RendererID{};
        GLenum m_InternalFormat = GL_RGBA8, m_DataFormat = GL_RGBA, m_DataType = GL_UNSIGNED_BYTE;
        Buffer m_Buffer;
    };

    class OpenGLTextureCube : public TextureCube
    {
    public:
        OpenGLTextureCube(const TextureCubeSpecification& spec);
        virtual ~OpenGLTextureCube();

        void Invalidate();

        uint32_t GetWidth() const override { return m_Width; };
        uint32_t GetHeight() const override { return m_Height; };
		RendererID GetRendererID() const override { return m_RendererID; };

        void SetData(void* data, uint32_t size) override;
        void ExtractBuffer() override;
        void CopyToHostBuffer(Buffer& buffer) const override;

        void Bind(uint32_t slot = 0) const;
        void Unbind() const;
    private:
        TextureCubeSpecification m_Spec;
        uint32_t m_Width, m_Height;
        RendererID m_RendererID{};
        void* m_LocalBuffer;
    };
}