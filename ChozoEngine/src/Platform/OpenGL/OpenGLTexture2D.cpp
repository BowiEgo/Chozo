#include "OpenGLTexture2D.h"

#include "stb_image.h"
#include <glad/glad.h>

namespace Chozo
{

    static GLenum GetGLFormat(const Texture2DFormat& format)
    {
        switch (format)
        {
            case Texture2DFormat::RED8UI: return GL_RED_INTEGER;
            case Texture2DFormat::RG8: return GL_RG8;
            case Texture2DFormat::RG16F: return GL_RG16F;
            case Texture2DFormat::RG32F: return GL_RG32F;
            case Texture2DFormat::RGB: return GL_RGB;
            case Texture2DFormat::RGBA: return GL_RGBA;
            case Texture2DFormat::RGBA16F: return GL_RGBA16F;
            case Texture2DFormat::RGBA32F: return GL_RGBA32F;
            case Texture2DFormat::B10R11G11UF: return GL_R11F_G11F_B10F;
            case Texture2DFormat::SRGB: return GL_SRGB;
            case Texture2DFormat::DEPTH32FSTENCIL8UINT: return GL_DEPTH32F_STENCIL8;
            case Texture2DFormat::DEPTH24STENCIL8: return GL_DEPTH24_STENCIL8;
            default: return GL_NONE;
        }
    }

    static GLenum GetGLParameter(const Texture2DParameter& param)
    {
        switch (param)
        {
            case Texture2DParameter::LINEAR: return GL_LINEAR;
            case Texture2DParameter::NEAREST: return GL_NEAREST;
            case Texture2DParameter::REPEAT: return GL_REPEAT;
            case Texture2DParameter::MIRRORED_REPEAT: return GL_MIRRORED_REPEAT;
            case Texture2DParameter::CLAMP_TO_EDGE: return GL_CLAMP_TO_EDGE;
            case Texture2DParameter::CLAMP_TO_BORDER: return GL_CLAMP_TO_BORDER;
            default: return GL_NONE;
        }
    }

    OpenGLTexture2D::OpenGLTexture2D(const Texture2DSpecification &spec)
        : m_Spec(spec), m_Width(spec.width), m_Height(spec.height)
    {
        glGenTextures(1, &m_RendererID);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetGLParameter(spec.minFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetGLParameter(spec.magFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetGLParameter(spec.wrapS));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetGLParameter(spec.wrapT));

        glTexImage2D(GL_TEXTURE_2D, 0, GetGLFormat(spec.Format), m_Width, m_Height, 0, GetGLFormat(spec.Format), GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    OpenGLTexture2D::OpenGLTexture2D(const std::string &path, const Texture2DSpecification &spec)
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

        glGenTextures(1, &m_RendererID);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetGLParameter(spec.minFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetGLParameter(spec.magFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetGLParameter(spec.wrapS));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetGLParameter(spec.wrapT));

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(data);
    }

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        glDeleteTextures(1, &m_RendererID);
    }

    void OpenGLTexture2D::Bind(uint32_t slot) const
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
    }

    void OpenGLTexture2D::SetData(const void *data, const uint32_t size)
    {
        Bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GetGLFormat(m_Spec.Format), m_Width, m_Height, 0, GetGLFormat(m_Spec.Format), GL_UNSIGNED_BYTE, data);
    }
}
