#include "OpenGLTexture.h"

#include "OpenGLUtils.h"

#include "stb_image.h"

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
            case ImageFormat::RED32F: return GL_RED;  // This is a floating-point format
            case ImageFormat::RG8: return GL_RG;
            case ImageFormat::RG16F: return GL_RG;
            case ImageFormat::RG32F: return GL_RG;
            case ImageFormat::RGB: return GL_RGB;
            case ImageFormat::RGB8: return GL_RGB;
            case ImageFormat::RGB16F: return GL_RGB;
            case ImageFormat::RGBA: return GL_RGBA;  // Should be GL_RGBA for 4 channels
            case ImageFormat::RGBA8: return GL_RGBA;
            case ImageFormat::RGBA16F: return GL_RGBA;
            case ImageFormat::RGBA32F: return GL_RGBA;
            case ImageFormat::B10R11G11UF: return GL_R11F_G11F_B10F; // Fine if using this format
            case ImageFormat::SRGB: return GL_SRGB_ALPHA;  // Use GL_SRGB_ALPHA for SRGB with alpha
            case ImageFormat::DEPTH32FSTENCIL8UINT: return GL_DEPTH_STENCIL;
            case ImageFormat::DEPTH24STENCIL8: return GL_DEPTH_STENCIL;
            default: return GL_NONE;
        }
    }

    static GLenum GetGLDataType(const ImageFormat& format)
    {
        switch (format)
        {
            case ImageFormat::RED8I: return GL_BYTE;  // Signed 8-bit integer
            case ImageFormat::RED8UI: return GL_UNSIGNED_BYTE;  // Unsigned 8-bit integer
            case ImageFormat::RED16UI: return GL_UNSIGNED_SHORT;  // Unsigned 16-bit integer
            case ImageFormat::RED32I: return GL_INT;  // Signed 32-bit integer
            case ImageFormat::RED32UI: return GL_UNSIGNED_INT;  // Unsigned 32-bit integer
            case ImageFormat::RED32F: return GL_FLOAT;  // 32-bit float
            case ImageFormat::RG8: return GL_UNSIGNED_BYTE;  // Unsigned 8-bit for two channels
            case ImageFormat::RG16F: return GL_HALF_FLOAT;  // 16-bit float (half float)
            case ImageFormat::RG32F: return GL_FLOAT;  // 32-bit float
            case ImageFormat::RGB: return GL_UNSIGNED_BYTE;  // Unsigned 8-bit
            case ImageFormat::RGB8: return GL_UNSIGNED_BYTE;  // Unsigned 8-bit
            case ImageFormat::RGB16F: return GL_FLOAT;  // 16-bit float
            case ImageFormat::RGBA: return GL_UNSIGNED_BYTE;  // Unsigned 8-bit for 4 channels
            case ImageFormat::RGBA8: return GL_UNSIGNED_BYTE;  // Unsigned 8-bit for 4 channels
            case ImageFormat::RGBA16F: return GL_HALF_FLOAT;  // 16-bit float for 4 channels
            case ImageFormat::RGBA32F: return GL_FLOAT;  // 32-bit float for 4 channels
            case ImageFormat::B10R11G11UF: return GL_UNSIGNED_INT_10F_11F_11F_REV;  // Special packed float format
            case ImageFormat::SRGB: return GL_UNSIGNED_BYTE;  // Unsigned 8-bit with sRGB
            case ImageFormat::DEPTH32FSTENCIL8UINT: return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;  // Depth + stencil combined
            case ImageFormat::DEPTH24STENCIL8: return GL_UNSIGNED_INT_24_8;  // 24-bit depth, 8-bit stencil
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
        m_InternalFormat = GetGLFormat(m_Spec.Format);
        m_DataFormat = GetGLDataFormat(m_Spec.Format);
        m_DataType = GetGLDataType(m_Spec.Format);

        glGenTextures(1, &m_RendererID); GCE;
        glBindTexture(GL_TEXTURE_2D, m_RendererID); GCE;

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetGLParameter(m_Spec.MinFilter)); GCE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetGLParameter(m_Spec.MagFilter)); GCE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GetGLParameter(m_Spec.WrapR)); GCE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetGLParameter(m_Spec.WrapS)); GCE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetGLParameter(m_Spec.WrapT)); GCE;

        glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Width, m_Height, 0, m_DataFormat, m_DataType, nullptr); GCE;
        glBindTexture(GL_TEXTURE_2D, 0); GCE;
    }

    OpenGLTexture2D::OpenGLTexture2D(const std::string &path, const TextureSpecification &spec)
        : m_Spec(spec), m_Path(path)
    {
        std::filesystem::path filePath = std::filesystem::path((char*)path.c_str());
        std::string fileExtension = filePath.extension().string();
        if (fileExtension == ".hdr")
            m_Spec.HDR = true;

        int width, height, channels;
        stbi_set_flip_vertically_on_load(1);

        void* m_DataBuffer;
        if (m_Spec.HDR)
            m_DataBuffer = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
        else
            m_DataBuffer = stbi_load(path.c_str(), &width, &height, &channels, 0);
        CZ_CORE_ASSERT(m_DataBuffer, "Failed to load image!");

        m_Width = width;
        m_Height = height;

        if (m_Spec.HDR)
            m_Spec.Format = ImageFormat::RGB16F;
        else if (channels == 4)
            m_Spec.Format = ImageFormat::RGBA8;
        else if (channels == 3)
            m_Spec.Format = ImageFormat::RGB8;
        else if (channels == 1)
            m_Spec.Format = ImageFormat::RED8UI;

        m_InternalFormat = GetGLFormat(m_Spec.Format);
        m_DataFormat = GetGLDataFormat(m_Spec.Format);
        m_DataType = GetGLDataType(m_Spec.Format);

        CZ_CORE_ASSERT(m_InternalFormat & m_DataFormat, "Format not supported!");

        glGenTextures(1, &m_RendererID); GCE;
        glBindTexture(GL_TEXTURE_2D, m_RendererID); GCE;

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetGLParameter(spec.MinFilter)); GCE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetGLParameter(spec.MagFilter)); GCE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetGLParameter(spec.WrapS)); GCE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetGLParameter(spec.WrapT)); GCE;

        glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, width, height, 0, m_DataFormat, m_DataType, m_DataBuffer); GCE;
        glBindTexture(GL_TEXTURE_2D, 0); GCE;

        stbi_image_free(m_DataBuffer); // TODO: maybe cause issue
    }

    OpenGLTexture2D::OpenGLTexture2D(const RendererID& id, const TextureSpecification &spec)
        : m_RendererID(id), m_Spec(spec), m_Width(spec.Width), m_Height(spec.Height)
    {
        m_InternalFormat = GetGLFormat(m_Spec.Format);
        m_DataFormat = GetGLDataFormat(m_Spec.Format);
        m_DataType = GetGLDataType(m_Spec.Format);
    }

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        // glDeleteTextures(1, &m_RendererID); GCE;
    }

    void OpenGLTexture2D::Resize(uint32_t width, uint32_t height)
    {
        if (m_Width != width || m_Height != height)
        {
            m_Spec.Width = width;
            m_Spec.Height = height;
            m_Width = width;
            m_Height = height;
            glBindTexture(GL_TEXTURE_2D, m_RendererID); GCE;
            glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, width, height, 0, m_DataFormat, m_DataType, m_DataBuffer); GCE;
        }
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
        glBindTexture(GL_TEXTURE_2D, m_RendererID); GCE;
        m_DataBuffer = (unsigned char*)data;
        glTexImage2D(GL_TEXTURE_2D, 0, GetGLFormat(m_Spec.Format), m_Width, m_Height, 0, GetGLDataFormat(m_Spec.Format), GetGLDataType(m_Spec.Format), m_DataBuffer); GCE;
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
