#include "OpenGLFramebuffer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Chozo {

    static const uint32_t s_MaxFramebufferSize = 8192;

    namespace Utils {

        static GLenum TextureTarget(bool multisampled)
        {
            return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        }

        static void CreateTexture(bool multisampled, RendererID* outID, uint32_t count)
        {
            // #ifdef GL_VERSION_4_4
            // glCreateTextures(TextureTarget(multisampled), count, outID);
            // #else
            glGenTextures(count, outID);
            // #endif
        }

        static void BindTexture(bool multisampled, RendererID id)
        {
            glBindTexture(TextureTarget(multisampled), id);
        }

        static void AttachColorTexture(RendererID id, int samples, GLenum internalformat, GLenum format, GLenum type, uint32_t width, uint32_t height, int index)
        {
            bool multisampled = samples > 1;
            if (multisampled)
            {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalformat, width, height, GL_FALSE);
            }
            else
            {
                glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, type, NULL);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
        }

        static void AttachDepthTexture(RendererID id, int samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height)
        {
            GLenum error = glGetError();
            bool multisampled = samples > 1;
            if (multisampled)
            {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
            }
            else
            {
                // #ifdef GL_VERSION_4_4
                // glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
                // #else
                glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
                // #endif
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0);
        }

        static bool IsDepthFormat(FramebufferTextureFormat format)
        {
            switch (format)
            {
                case FramebufferTextureFormat::DEPTH24STENCIL8: return true;
                default: return false;
            }

            return false;
        }
    }

    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification &spec)
        : m_Specification(spec)
    {
        for (auto format : m_Specification.Attachments.Attachments)
        {
            if (!Utils::IsDepthFormat(format.TextureFormat))
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

    void OpenGLFramebuffer::Bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
        glViewport(0, 0, m_Specification.Width, m_Specification.Height);
    }

    void OpenGLFramebuffer::Unbind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFramebuffer::Resize(float& width, float& height)
    {
        if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
        {
            CZ_CORE_WARN("Attempted to resize framebuffer to {0}, {1}", width, height);
            return;
        }
        m_Specification.Width = width;
        m_Specification.Height = height;

        Invalidate();
    }

    int OpenGLFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
    {
        CZ_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "attachmentIndex is smaller than colorAttachments size");
        glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
        int pixelData;
        glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
        return pixelData;
    }

    void OpenGLFramebuffer::ClearColorAttachmentBuffer(uint32_t attachmentIndex, const void* value)
    {
        CZ_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "attachmentIndex is smaller than colorAttachments size");
        Bind();
        glBindTexture(GL_TEXTURE_2D, m_ColorAttachments[attachmentIndex]);
        std::vector<int> clearValues(m_Specification.Width * m_Specification.Height, -1);
        // // #ifdef GL_VERSION_4_4
        // // glClearTexImage(m_IDAttachment, 0, GL_RED_INTEGER, GL_INT, &clearValues);
        // // #else
        switch (m_ColorAttachmentSpecs[attachmentIndex].TextureFormat)
        {
            case FramebufferTextureFormat::RGBA8:
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Specification.Width, m_Specification.Height, GL_RGB, GL_UNSIGNED_BYTE, clearValues.data());
                break;
            case FramebufferTextureFormat::RED_INTEGER:
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Specification.Width, m_Specification.Height, GL_RED_INTEGER, GL_INT, clearValues.data());
                break;
            default:
                break;
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void OpenGLFramebuffer::Invalidate()
    {
        if (m_RendererID)
        {
            Release();
        }

        glGenFramebuffers(1, &m_RendererID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

        bool multisampled = m_Specification.Samples > 1;

        // Attachments
        if (m_ColorAttachmentSpecs.size())
        {
            m_ColorAttachments.resize(m_ColorAttachmentSpecs.size());
            Utils::CreateTexture(multisampled, m_ColorAttachments.data(), m_ColorAttachments.size());

            for (size_t i = 0; i < m_ColorAttachmentSpecs.size(); i++)
            {
                Utils::BindTexture(multisampled, m_ColorAttachments[i]);
                switch (m_ColorAttachmentSpecs[i].TextureFormat)
                {
                    case FramebufferTextureFormat::RGBA8:
                        Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGBA8, GL_RGB, GL_UNSIGNED_BYTE, m_Specification.Width, m_Specification.Height, i);
                        break;
                    case FramebufferTextureFormat::RED_INTEGER:
                        Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_R32I, GL_RED_INTEGER, GL_INT, m_Specification.Width, m_Specification.Height, i);
                        break;
                    default:
                        break;
                }
            }
        }

        if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None)
        {
            Utils::CreateTexture(multisampled, &m_DepthAttachment, 1);
            Utils::BindTexture(multisampled, m_DepthAttachment);
            switch (m_DepthAttachmentSpec.TextureFormat)
            {
                case FramebufferTextureFormat::DEPTH24STENCIL8:
                    Utils::AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
                    break;
                default:
                    break;
            }
        }

        if (m_ColorAttachments.size() > 1)
        {
            CZ_CORE_ASSERT(m_ColorAttachments.size() <= 4, "");
            GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
            glDrawBuffers(m_ColorAttachments.size(), buffers);
        }
        else if (m_ColorAttachments.empty())
        {
            // Only depth-pass
            glDrawBuffer(GL_NONE);
        }

        CZ_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");
    }

    void OpenGLFramebuffer::Release()
    {
        glDeleteFramebuffers(1, &m_RendererID);
        glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
        glDeleteRenderbuffers(1, &m_DepthAttachment);

        m_ColorAttachments.clear();
        m_DepthAttachment = 0;
    }
}