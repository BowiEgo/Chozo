#include "OpenGLTexture.h"

#include "OpenGLUtils.h"

#include "stb_image.h"
#include <glad/glad.h>

namespace Chozo
{

    static GLenum GetGLFormat(const ImageFormat& format)
    {
        switch (format)
        {
            case ImageFormat::RED8I: return GL_R8I;
            case ImageFormat::RED8UI: return GL_R8UI;
            case ImageFormat::RED16I: return GL_R16I;
            case ImageFormat::RED16UI: return GL_R16UI;
            case ImageFormat::RED32I: return GL_R32I;
            case ImageFormat::RED32UI: return GL_R32UI;
            case ImageFormat::RED32F: return GL_R32F;
            case ImageFormat::RG8: return GL_RG8;
            case ImageFormat::RG16F: return GL_RG16F;
            case ImageFormat::RG32F: return GL_RG32F;
            case ImageFormat::RGB: return GL_RGB;
            case ImageFormat::RGB8: return GL_RGB8;
            case ImageFormat::RGB16F: return GL_RGB16F;
            case ImageFormat::RGBA: return GL_RGBA;
            case ImageFormat::RGBA8: return GL_RGBA8;
            case ImageFormat::RGBA16F: return GL_RGBA16F;
            case ImageFormat::RGBA32F: return GL_RGBA32F;
            case ImageFormat::B10R11G11UF: return GL_R11F_G11F_B10F;
            case ImageFormat::SRGB: return GL_SRGB;
            case ImageFormat::DEPTH32FSTENCIL8UINT: return GL_DEPTH32F_STENCIL8;
            case ImageFormat::DEPTH24STENCIL8: return GL_DEPTH24_STENCIL8;
            default: return GL_NONE;
        }
    }

    static GLenum GetGLDataFormat(const ImageFormat& format)
    {
        switch (format)
        {
            case ImageFormat::RED8I: return GL_RED_INTEGER;
            case ImageFormat::RED8UI: return GL_RED_INTEGER;
            case ImageFormat::RED16UI: return GL_RED_INTEGER;
            case ImageFormat::RED32I: return GL_RED_INTEGER;
            case ImageFormat::RED32UI: return GL_RED_INTEGER;
            case ImageFormat::RED32F: return GL_RED_INTEGER;
            case ImageFormat::RG8: return GL_RG;
            case ImageFormat::RG16F: return GL_RG;
            case ImageFormat::RG32F: return GL_RG;
            case ImageFormat::RGB: return GL_RGB;
            case ImageFormat::RGB8: return GL_RGB;
            case ImageFormat::RGB16F: return GL_RGB;
            case ImageFormat::RGBA: return GL_RGB;
            case ImageFormat::RGBA8: return GL_RGB;
            case ImageFormat::RGBA16F: return GL_RGB;
            case ImageFormat::RGBA32F: return GL_RGB;
            case ImageFormat::B10R11G11UF: return GL_R11F_G11F_B10F; // TODO: fix
            case ImageFormat::SRGB: return GL_SRGB;  // TODO: fix
            case ImageFormat::DEPTH32FSTENCIL8UINT: return GL_DEPTH32F_STENCIL8;  // TODO: fix
            case ImageFormat::DEPTH24STENCIL8: return GL_DEPTH24_STENCIL8;  // TODO: fix
            default: return GL_NONE;
        }
    }

    static GLenum GetGLDataType(const ImageFormat& format)
    {
        switch (format)
        {
            case ImageFormat::RED8UI: return GL_INT;
            case ImageFormat::RED16UI: return GL_INT;
            case ImageFormat::RED32I: return GL_INT;
            case ImageFormat::RED32UI: return GL_INT;
            case ImageFormat::RED32F: return GL_FLOAT;
            case ImageFormat::RG8: return GL_UNSIGNED_BYTE;
            case ImageFormat::RG16F: return GL_UNSIGNED_BYTE;
            case ImageFormat::RG32F: return GL_UNSIGNED_BYTE;
            case ImageFormat::RGB: return GL_UNSIGNED_BYTE;
            case ImageFormat::RGB8: return GL_UNSIGNED_BYTE;
            case ImageFormat::RGB16F: return GL_UNSIGNED_BYTE;
            case ImageFormat::RGBA: return GL_UNSIGNED_BYTE;
            case ImageFormat::RGBA8: return GL_UNSIGNED_BYTE;
            case ImageFormat::RGBA16F: return GL_UNSIGNED_BYTE;
            case ImageFormat::RGBA32F: return GL_UNSIGNED_BYTE;
            case ImageFormat::B10R11G11UF: return GL_FLOAT; // TODO: fix
            case ImageFormat::SRGB: return GL_UNSIGNED_BYTE;  // TODO: fix
            case ImageFormat::DEPTH32FSTENCIL8UINT: return GL_UNSIGNED_BYTE;  // TODO: fix
            case ImageFormat::DEPTH24STENCIL8: return GL_UNSIGNED_BYTE;  // TODO: fix
            default: return GL_NONE;
        }
    }

    static GLenum GetGLParameter(const ImageParameter& param)
    {
        switch (param)
        {
            case ImageParameter::LINEAR: return GL_LINEAR;
            case ImageParameter::NEAREST: return GL_NEAREST;
            case ImageParameter::REPEAT: return GL_REPEAT;
            case ImageParameter::MIRRORED_REPEAT: return GL_MIRRORED_REPEAT;
            case ImageParameter::CLAMP_TO_EDGE: return GL_CLAMP_TO_EDGE;
            case ImageParameter::CLAMP_TO_BORDER: return GL_CLAMP_TO_BORDER;
            default: return GL_NONE;
        }
    }

    OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification &spec)
        : m_Spec(spec), m_Width(spec.Width), m_Height(spec.Height)
    {
        GLenum internalFormat = GetGLFormat(m_Spec.Format);
        GLenum dataFormat = GetGLDataFormat(m_Spec.Format);
        GLenum dataType = GetGLDataType(m_Spec.Format);

        glGenTextures(1, &m_RendererID); GCE;
        glBindTexture(GL_TEXTURE_2D, m_RendererID); GCE;

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetGLParameter(m_Spec.MinFilter)); GCE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetGLParameter(m_Spec.MagFilter)); GCE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GetGLParameter(m_Spec.WrapR)); GCE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetGLParameter(m_Spec.WrapS)); GCE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetGLParameter(m_Spec.WrapT)); GCE;

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, dataFormat, dataType, nullptr); GCE;
        glBindTexture(GL_TEXTURE_2D, 0); GCE;

    }

    OpenGLTexture2D::OpenGLTexture2D(const std::string &path, const TextureSpecification &spec)
        : m_Spec(spec), m_Path(path)
    {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(1);
        stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        CZ_CORE_ASSERT(data, "Failed to load image!");
        m_Width = width;
        m_Height = height;

        GLenum internalFormat = 0, dataFormat = 0;
        if (channels == 4)
        {
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
        }
        else if (channels == 3)
        {
            internalFormat = GL_RGB8;
            dataFormat = GL_RGB;
        }

        CZ_CORE_ASSERT(internalFormat & dataFormat, "Format not supported!");

        glGenTextures(1, &m_RendererID); GCE;
        glBindTexture(GL_TEXTURE_2D, m_RendererID); GCE;

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetGLParameter(spec.MinFilter)); GCE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetGLParameter(spec.MagFilter)); GCE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetGLParameter(spec.WrapS)); GCE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetGLParameter(spec.WrapT)); GCE;

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data); GCE;
        glBindTexture(GL_TEXTURE_2D, 0); GCE;

        stbi_image_free(data);
    }

    OpenGLTexture2D::OpenGLTexture2D(const RendererID& id, const TextureSpecification &spec)
        : m_RendererID(id), m_Spec(spec), m_Width(spec.Width), m_Height(spec.Height)
    {
    }

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        // glDeleteTextures(1, &m_RendererID); GCE;
    }

    void OpenGLTexture2D::Bind(uint32_t slot) const
    {
        glActiveTexture(GL_TEXTURE0 + slot); GCE;
        glBindTexture(GL_TEXTURE_2D, m_RendererID); GCE;
    }

    void OpenGLTexture2D::Unbind() const
    {
        glBindTexture(GL_TEXTURE_2D, 0); GCE;
    }

    void OpenGLTexture2D::SetData(const void *data, const uint32_t size)
    {
        Bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GetGLFormat(m_Spec.Format), m_Width, m_Height, 0, GetGLDataFormat(m_Spec.Format), GetGLDataType(m_Spec.Format), data); GCE;
    }

    //////////////////////////////////////////////////////////
    //---------------------Texture Cube---------------------//
    //////////////////////////////////////////////////////////
    OpenGLTextureCube::OpenGLTextureCube(const TextureSpecification& spec)
        : m_Spec(spec), m_Width(spec.Width), m_Height(spec.Height)
    {
        Invalidate();
    }

    OpenGLTextureCube::~OpenGLTextureCube()
    {
    }

    void OpenGLTextureCube::Invalidate()
    {
        if (m_RendererID)
        {
            glDeleteTextures(1, &m_RendererID); GCE;
        }

        glGenTextures(1, &m_RendererID); GCE;
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID); GCE;
        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, m_Spec.Width, m_Spec.Height, 0, GL_RGB, GL_FLOAT, nullptr); GCE;
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); GCE;
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); GCE;
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); GCE;
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR); GCE;
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR); GCE;
    }

    void OpenGLTextureCube::Bind(uint32_t slot) const
    {
        glActiveTexture(GL_TEXTURE0 + slot); GCE;
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID); GCE;
    }

    void OpenGLTextureCube::Unbind() const
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0); GCE;
    }

    void OpenGLTextureCube::SetData(void* data, uint32_t size)
    {
        m_LocalBuffer = data;

        glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID); GCE;
        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GetGLFormat(m_Spec.Format), m_Spec.Width, m_Spec.Height, 0, GetGLDataFormat(m_Spec.Format), GL_FLOAT, static_cast<float*>(data) + (size / 6) * i); GCE;
        }
    }
}
