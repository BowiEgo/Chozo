#include "OpenGLTexture.h"

#include "stb_image.h"
#include <glad/glad.h>

namespace Chozo
{

    static GLenum GetGLFormat(const ImageFormat& format)
    {
        switch (format)
        {
            case ImageFormat::RED8UI: return GL_RED_INTEGER;
            case ImageFormat::RG8: return GL_RG8;
            case ImageFormat::RG16F: return GL_RG16F;
            case ImageFormat::RG32F: return GL_RG32F;
            case ImageFormat::RGB: return GL_RGB;
            case ImageFormat::RGBA: return GL_RGBA;
            case ImageFormat::RGBA16F: return GL_RGBA16F;
            case ImageFormat::RGBA32F: return GL_RGBA32F;
            case ImageFormat::B10R11G11UF: return GL_R11F_G11F_B10F;
            case ImageFormat::SRGB: return GL_SRGB;
            case ImageFormat::DEPTH32FSTENCIL8UINT: return GL_DEPTH32F_STENCIL8;
            case ImageFormat::DEPTH24STENCIL8: return GL_DEPTH24_STENCIL8;
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
        glGenTextures(1, &m_RendererID);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetGLParameter(spec.MinFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetGLParameter(spec.MagFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetGLParameter(spec.WrapS));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetGLParameter(spec.WrapT));

        glTexImage2D(GL_TEXTURE_2D, 0, GetGLFormat(spec.Format), m_Width, m_Height, 0, GetGLFormat(spec.Format), GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
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

        glGenTextures(1, &m_RendererID);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetGLParameter(spec.MinFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetGLParameter(spec.MagFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetGLParameter(spec.WrapS));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetGLParameter(spec.WrapT));

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(data);
    }

    OpenGLTexture2D::OpenGLTexture2D(const RendererID &id, const TextureSpecification &spec)
        : m_RendererID(id), m_Spec(spec), m_Width(spec.Width), m_Height(spec.Height)
    {
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

    void OpenGLTexture2D::Unbind() const
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void OpenGLTexture2D::SetData(const void *data, const uint32_t size)
    {
        Bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GetGLFormat(m_Spec.Format), m_Width, m_Height, 0, GetGLFormat(m_Spec.Format), GL_UNSIGNED_BYTE, data);
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
            glDeleteTextures(1, &m_RendererID);
        }

        glGenTextures(1, &m_RendererID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, m_Spec.Width, m_Spec.Height, 0, GL_RGB, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    void OpenGLTextureCube::Bind(uint32_t slot) const
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
    }

    void OpenGLTextureCube::Unbind() const
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
}
