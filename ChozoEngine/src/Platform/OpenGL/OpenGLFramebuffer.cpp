#include "OpenGLFramebuffer.h"

#include "OpenGLUtils.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Chozo {

    static const uint32_t s_MaxFramebufferSize = 8192;

    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification &spec)
        : m_Specification(spec)
    {
        for (auto format : m_Specification.Attachments.Attachments)
        {
            if (!Image::IsDepthFormat(format.TextureFormat))
                m_ColorAttachmentSpecs.emplace_back(format);
            else
                m_DepthAttachmentSpec = format;
        }

		Invalidate();
    }

    OpenGLFramebuffer::~OpenGLFramebuffer()
    {
        Release();
    }

    void OpenGLFramebuffer::Invalidate()
    {
        Release();

        glGenFramebuffers(1, &m_RendererID); GCE;
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID); GCE;

        bool multisampled = m_Specification.Samples > 1;

        // Attachments
        if (m_ColorAttachmentSpecs.size())
            CreateColorAttachmentImages(m_Specification.Samples, m_ColorAttachmentSpecs, m_Specification.Width, m_Specification.Height);

        if (m_DepthAttachmentSpec.TextureFormat != ImageFormat::None)
        {
            if (m_Specification.DepthRenderbuffer)
            {
                glGenRenderbuffers(1, &m_DepthAttachment); GCE;
                glBindRenderbuffer(GL_RENDERBUFFER, m_DepthAttachment); GCE;
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_Specification.Width, m_Specification.Height); GCE;
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthAttachment); GCE;
            }
            else
                CreateDepthAttachmentImage(m_Specification.Samples, m_DepthAttachmentSpec, m_Specification.Width, m_Specification.Height);
        }

        if (m_ColorAttachments.size() > 1)
        {
            CZ_CORE_ASSERT(m_ColorAttachments.size() <= 8, "");
            GLenum buffers[8] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7 };
            glDrawBuffers(m_ColorAttachments.size(), buffers); GCE;
        }
        else if (m_ColorAttachments.empty())
        {
            // Only depth-pass
            glDrawBuffer(GL_NONE); GCE;
        }

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            CZ_CORE_ERROR("Framebuffer is incomplete! Status: {0}", status);
            CZ_CORE_ASSERT(false, "");
        }
    }

    void OpenGLFramebuffer::Release()
    {
        if (m_RendererID)
        {
            glDeleteFramebuffers(1, &m_RendererID); GCE;
            m_RendererID = 0;
        }
        
        if (!m_ColorAttachments.empty())
        {
            glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data()); GCE;
            m_ColorAttachments.clear();
            m_ColorAttachmentImages.clear();
        }
        
        if (m_DepthAttachment)
        {
            if (m_Specification.DepthRenderbuffer)
            {
                glDeleteRenderbuffers(1, &m_DepthAttachment); GCE;
            }
            else
            {
                glDeleteTextures(1, &m_DepthAttachment); GCE;
            }

            m_DepthAttachment = 0;
            m_DepthAttachmentImage = nullptr;

        }
    }

    void OpenGLFramebuffer::Bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID); GCE;
        glViewport(0, 0, m_Specification.Width, m_Specification.Height); GCE;
    }

    void OpenGLFramebuffer::Unbind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0); GCE;
    }

    void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height, int mip)
    {
        if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
        {
            CZ_CORE_WARN("Attempted to resize framebuffer to {0}, {1}", width, height);
            return;
        }

        if (m_Specification.Width != width || m_Specification.Height != height)
        {
            m_Specification.Width = width;
            m_Specification.Height = height;

            Bind();
            for (auto& images : m_ColorAttachmentImages)
                images->Resize(width, height);

            if (m_DepthAttachmentImage)
            {
                if (m_Specification.DepthRenderbuffer)
                {
                    glBindRenderbuffer(GL_RENDERBUFFER, m_DepthAttachment); GCE;
                    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height); GCE;
                } else {
                    m_DepthAttachmentImage->Resize(width, height);
                    if (mip > -1)
                        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, mip);
                }
            }
            // Unbind();
        }
    }

    int OpenGLFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
    {
        Bind();
        CZ_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "attachmentIndex is larger than colorAttachments size");
        glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex); GCE;
        int pixelData;
        glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData); GCE;
        Unbind();
        return pixelData;
    }
#if 0
    void OpenGLFramebuffer::ClearColorAttachmentBuffer(uint32_t attachmentIndex)
    {
        CZ_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "attachmentIndex is larger than colorAttachments size");
        std::vector<int> intClearValues = std::vector(m_Specification.Width * m_Specification.Height, -1);
        Bind();
        glBindTexture(GL_TEXTURE_2D, m_ColorAttachments[attachmentIndex]); GCE;
        // // #ifdef GL_VERSION_4_4
        // // glClearTexImage(m_IDAttachment, 0, GL_RED_INTEGER, GL_INT, &clearValues);
        // // #else
        switch (m_ColorAttachmentSpecs[attachmentIndex].TextureFormat)
        {
            case ImageFormat::RGBA8:
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Specification.Width, m_Specification.Height, GL_RGB, GL_UNSIGNED_BYTE, intClearValues.data()); GCE;
                break;
            case ImageFormat::RGBA32F:
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Specification.Width, m_Specification.Height, GL_RGB, GL_UNSIGNED_BYTE, intClearValues.data()); GCE;
                break;
            case ImageFormat::RED32I:
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Specification.Width, m_Specification.Height, GL_RED_INTEGER, GL_INT, intClearValues.data()); GCE;
                break;
            default:
                break;
        }
        glBindTexture(GL_TEXTURE_2D, 0); GCE;
        Unbind();
    }
#endif

    // TODO: fix issue
    void OpenGLFramebuffer::CreateColorAttachmentImages(int samples, std::vector<FramebufferTexture2DSpecification> attachmentSpecs, uint32_t width, uint32_t height)
    {
        uint32_t count = attachmentSpecs.size();
        bool multisampled = samples > 1;
        GLenum target = multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

        m_ColorAttachments.resize(count);
        m_ColorAttachmentImages.resize(count);

        for (size_t i = 0; i < m_Specification.ExistingImages.size(); i++)
        {
            m_ColorAttachments[i] = m_Specification.ExistingImages[i]->GetRendererID();
            m_ColorAttachmentImages[i] = m_Specification.ExistingImages[i];
        }

        size_t existCount = m_Specification.ExistingImages.size();
        count = m_ColorAttachments.size() - existCount;

        for (size_t i = existCount; i < count; i++)
        {
            Texture2DSpecification spec;
            spec.Samples = m_Specification.Samples;
            spec.Format = m_ColorAttachmentSpecs[i].TextureFormat;
            spec.Width = m_Specification.Width;
            spec.Height = m_Specification.Height;
            if (m_ColorAttachmentSpecs[i].TextureFormat == ImageFormat::RED32I)
            {
                spec.MinFilter = ImageParameter::NEAREST;
                spec.MagFilter = ImageParameter::NEAREST;
            }

            Ref<Texture2D> texture = Texture2D::Create(spec);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, target, texture->GetRendererID(), 0); GCE;

            m_ColorAttachments[i] = texture->GetRendererID();
            m_ColorAttachmentImages[i] = texture;
        }
    }

    void OpenGLFramebuffer::CreateDepthAttachmentImage(int samples, FramebufferTexture2DSpecification attachmentSpec, uint32_t width, uint32_t height)
    {
        bool multisampled = samples > 1;
        GLenum target = multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

        Texture2DSpecification spec;
        spec.Samples = m_Specification.Samples;
        spec.Format = m_DepthAttachmentSpec.TextureFormat;
        spec.Width = m_Specification.Width;
        spec.Height = m_Specification.Height;
        spec.MinFilter = ImageParameter::LINEAR;
        spec.MagFilter = ImageParameter::LINEAR;

        m_DepthAttachmentImage = Texture2D::Create(spec);
        m_DepthAttachment = m_DepthAttachmentImage->GetRendererID();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, target, m_DepthAttachment, 0); GCE;
    }
}