#include "OpenGLTexture.h"

#include "OpenGLUtils.h"
#include "Chozo/FileSystem/TextureImporter.h"

#include "stb_image.h"

namespace Chozo
{

    //==============================================================================
	///  OpenGLTexture2D
    OpenGLTexture2D::OpenGLTexture2D(const Texture2DSpecification &spec)
        : m_Spec(spec), m_Width(spec.Width), m_Height(spec.Height)
    {
        Invalidate();
    }

    OpenGLTexture2D::OpenGLTexture2D(const std::string &path, const Texture2DSpecification &spec)
        : m_Spec(spec), m_Path(path)
    {
        m_Buffer = TextureImporter::ToBufferFromFile(path, m_Spec.Format, m_Width, m_Height);

        Invalidate();
    }

    OpenGLTexture2D::OpenGLTexture2D(const RendererID& id, const Texture2DSpecification &spec)
        : m_RendererID(id), m_Spec(spec), m_Width(spec.Width), m_Height(spec.Height)
    {
        m_InternalFormat = GetGLFormat(m_Spec.Format);
        m_DataFormat = GetGLDataFormat(m_Spec.Format);
        m_DataType = GetGLDataType(m_Spec.Format);
    }

    OpenGLTexture2D::OpenGLTexture2D(Buffer buffer, const Texture2DSpecification &spec)
        : m_Spec(spec), m_Width(spec.Width), m_Height(spec.Height)
    {
        if (m_Buffer.Size != 0)
            m_Buffer.Release();

        m_Buffer = Buffer::Copy(buffer);
        Invalidate();
    }

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        glDeleteTextures(1, &m_RendererID); GCE;
        if (m_Buffer.Size != 0)
            m_Buffer.Release();
    }

    void OpenGLTexture2D::Resize(uint32_t width, uint32_t height)
    {
        if (m_Width != width || m_Height != height)
        {
            // int channel = GetChannelCount(m_Spec.Format);

            // m_DataBuffer = new unsigned char[m_Spec.Width * m_Spec.Height * channel * sizeof(float)];
            m_Spec.Width = width;
            m_Spec.Height = height;
            m_Width = width;
            m_Height = height;
            glBindTexture(GL_TEXTURE_2D, m_RendererID); GCE;
            glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, width, height, 0, m_DataFormat, m_DataType, m_Buffer.Data); GCE;
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
        m_Buffer.Allocate(size);
        m_Buffer.Write(data, size);
        glBindTexture(GL_TEXTURE_2D, m_RendererID); GCE;
        glTexImage2D(GL_TEXTURE_2D, 0, GetGLFormat(m_Spec.Format), m_Width, m_Height, 0, GetGLDataFormat(m_Spec.Format), GetGLDataType(m_Spec.Format), m_Buffer.Data); GCE;
    }

    void OpenGLTexture2D::CopyToHostBuffer(Buffer& buffer)
    {
        if (m_Buffer.Size != 0)
        {
            m_Buffer.CopyTo(buffer);
            return;
        }

        uint64_t size = m_Width * m_Height * Image::GetBytesPerPixel(m_Spec.Format);
        m_Buffer.Allocate(size);

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
        glGetTexImage(GL_TEXTURE_2D, 0, m_DataFormat, m_DataType, m_Buffer.Data);
        glBindTexture(GL_TEXTURE_2D, 0);

        if (m_Buffer.Size != 0)
            m_Buffer.CopyTo(buffer);
        else
            CZ_CORE_WARN("Texture buffer data is empty.");
    }

    void OpenGLTexture2D::Invalidate()
    {
        m_InternalFormat = GetGLFormat(m_Spec.Format);
        m_DataFormat = GetGLDataFormat(m_Spec.Format);
        m_DataType = GetGLDataType(m_Spec.Format);

        CZ_CORE_ASSERT(m_InternalFormat & m_DataFormat, "Format not supported!");

        glGenTextures(1, &m_RendererID); GCE;
        glBindTexture(GL_TEXTURE_2D, m_RendererID); GCE;

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetGLParameter(m_Spec.MinFilter)); GCE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetGLParameter(m_Spec.MagFilter)); GCE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetGLParameter(m_Spec.WrapS)); GCE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetGLParameter(m_Spec.WrapT)); GCE;

        glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Width, m_Height, 0, m_DataFormat, m_DataType, m_Buffer.Data); GCE;
        glBindTexture(GL_TEXTURE_2D, 0); GCE;
    }

    //==============================================================================
	/// OpenGLTextureCube
    OpenGLTextureCube::OpenGLTextureCube(const TextureCubeSpecification& spec)
        : m_Spec(spec), m_Width(spec.Width), m_Height(spec.Height)
    {
        Invalidate();
    }

    OpenGLTextureCube::~OpenGLTextureCube()
    {
    }

    void OpenGLTextureCube::Invalidate()
    {
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); GCE;

        if (m_RendererID)
        {
            glDeleteTextures(1, &m_RendererID); GCE;
        }

        glGenTextures(1, &m_RendererID); GCE;
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID); GCE;
        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GetGLFormat(m_Spec.Format), m_Spec.Width, m_Spec.Height, 0, GetGLDataFormat(m_Spec.Format), GetGLDataType(m_Spec.Format), nullptr); GCE;
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GetGLParameter(m_Spec.WrapS)); GCE;
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GetGLParameter(m_Spec.WrapT)); GCE;
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GetGLParameter(m_Spec.WrapR)); GCE;
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GetGLParameter(m_Spec.MinFilter)); GCE;
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GetGLParameter(m_Spec.MagFilter)); GCE;

        if (m_Spec.Mipmap)
            glGenerateMipmap(GL_TEXTURE_CUBE_MAP); GCE;
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

    void OpenGLTextureCube::CopyToHostBuffer(Buffer& buffer)
    {
    }
}
