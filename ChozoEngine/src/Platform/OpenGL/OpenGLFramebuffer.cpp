#include "OpenGLFramebuffer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Chozo {
    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification &spec)
        : m_Specification(spec)
    {
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
        m_Specification.Width = width;
        m_Specification.Height = height;

        Invalidate();
    }

    void OpenGLFramebuffer::ClearIDBuffer()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
        glBindTexture(GL_TEXTURE_2D, m_IDAttachment);
        std::vector<int> clearValues(m_Specification.Width * m_Specification.Height, -1);
        // #ifdef GL_VERSION_4_4
        // glClearTexImage(m_IDAttachment, 0, GL_RED_INTEGER, GL_INT, &clearValues);
        // #else
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Specification.Width, m_Specification.Height, GL_RED_INTEGER, GL_INT, clearValues.data());
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

        GLenum internalFormat = m_Specification.HDR ? GL_RGB16 : GL_RGB, dataFormat = GL_RGB;
        glGenTextures(1, &m_ColorAttachment);
        glBindTexture(GL_TEXTURE_2D, m_ColorAttachment);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Specification.Width, m_Specification.Height, 0, dataFormat, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0);
        
        glGenTextures(1, &m_IDAttachment);
        glBindTexture(GL_TEXTURE_2D, m_IDAttachment);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, m_Specification.Width, m_Specification.Height, 0, GL_RED_INTEGER, GL_INT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_IDAttachment, 0);

        GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, drawBuffers);
        
        glGenRenderbuffers(1, &m_DepthAttachment);
        glBindRenderbuffer(GL_RENDERBUFFER, m_DepthAttachment);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Specification.Width, m_Specification.Height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthAttachment);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        CZ_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");
    }

    void OpenGLFramebuffer::Release()
    {
        glDeleteFramebuffers(1, &m_RendererID);
        glDeleteTextures(1, &m_ColorAttachment);
        glDeleteTextures(1, &m_IDAttachment);
        glDeleteRenderbuffers(1, &m_DepthAttachment);
    }
}