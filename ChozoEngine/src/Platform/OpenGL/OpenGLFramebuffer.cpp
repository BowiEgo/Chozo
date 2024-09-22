#include "OpenGLFramebuffer.h"

#include "OpenGLUtils.h"

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
            glGenTextures(count, outID); GCE;
            // #endif
        }

        static void BindTexture(bool multisampled, RendererID id)
        {
            glBindTexture(TextureTarget(multisampled), id); GCE;
        }

        static void AttachColorTexture(Ref<Texture2D> texture, int index)
        {
            RendererID id = texture->GetRendererID();
            bool multisampled = texture->GetSpecification().Samples > 1;

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0); GCE;
        }

        static void AttachColorTexture(RendererID id, int samples, int index)
        {
            bool multisampled = samples > 1;

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0); GCE;
        }

        static void AttachColorTexture(RendererID id, TextureSpecification spec, int index)
        {
            bool multisampled = spec.Samples > 1;

            GLenum internalformat = GetGLFormat(spec.Format);
            GLenum dataFormat = GetGLDataFormat(spec.Format);
            GLenum dataType = GetGLDataType(spec.Format);

            if (multisampled)
            {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, spec.Samples, internalformat, spec.Width, spec.Height, GL_FALSE); GCE;
            }
            else
            {
                glTexImage2D(GL_TEXTURE_2D, 0, internalformat, spec.Width, spec.Height, 0, dataFormat, dataType, NULL); GCE;
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetGLParameter(spec.MinFilter)); GCE;
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetGLParameter(spec.MagFilter)); GCE;
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetGLParameter(spec.WrapS)); GCE;
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetGLParameter(spec.WrapT)); GCE;
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0); GCE;
        }

        static void AttachColorTexture(RendererID id, int samples, GLenum internalformat, GLenum format, GLenum type, uint32_t width, uint32_t height, int index)
        {
            bool multisampled = samples > 1;
            if (multisampled)
            {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalformat, width, height, GL_FALSE); GCE;
            }
            else
            {
                glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, type, NULL); GCE;
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); GCE;
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); GCE;
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); GCE;
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); GCE;
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0); GCE;
        }

        static void AttachDepthTexture(RendererID id, int samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height)
        {
            GLenum error = glGetError();
            bool multisampled = samples > 1;
            if (multisampled)
            {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE); GCE;
            }
            else
            {
                // #ifdef GL_VERSION_4_4
                // glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
                // #else
                glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL); GCE;
                // #endif
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); GCE;
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); GCE;
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER); GCE;
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); GCE;
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER); GCE;
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0); GCE;
        }

        static bool IsDepthFormat(ImageFormat format)
        {
            switch (format)
            {
                case ImageFormat::DEPTH24STENCIL8: return true;
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
            m_IntClearValues = std::vector(width * height, -1);
            CZ_CORE_INFO("{}, {}: {}", width, height, m_IntClearValues.size());
            for (auto images : m_ColorAttachmentImages)
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
            glViewport(0, 0, width, height); GCE;
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

    void OpenGLFramebuffer::ClearColorAttachmentBuffer(uint32_t attachmentIndex)
    {
        CZ_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "attachmentIndex is larger than colorAttachments size");
        Bind();
        glBindTexture(GL_TEXTURE_2D, m_ColorAttachments[attachmentIndex]); GCE;
        // // #ifdef GL_VERSION_4_4
        // // glClearTexImage(m_IDAttachment, 0, GL_RED_INTEGER, GL_INT, &clearValues);
        // // #else
        switch (m_ColorAttachmentSpecs[attachmentIndex].TextureFormat)
        {
            case ImageFormat::RGBA8:
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Specification.Width, m_Specification.Height, GL_RGB, GL_UNSIGNED_BYTE, m_IntClearValues.data()); GCE;
                break;
            case ImageFormat::RGBA32F:
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Specification.Width, m_Specification.Height, GL_RGB, GL_UNSIGNED_BYTE, m_IntClearValues.data()); GCE;
                break;
            case ImageFormat::RED32I:
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Specification.Width, m_Specification.Height, GL_RED_INTEGER, GL_INT, m_IntClearValues.data()); GCE;
                break;
            default:
                break;
        }
        glBindTexture(GL_TEXTURE_2D, 0); GCE;
        Unbind();
    }

    void OpenGLFramebuffer::Invalidate()
    {
        Release();

        glGenFramebuffers(1, &m_RendererID); GCE;
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID); GCE;

        bool multisampled = m_Specification.Samples > 1;

        // Attachments
        if (m_ColorAttachmentSpecs.size())
        {
#if 0
            CreateTextures(m_Specification.Samples, m_ColorAttachmentSpecs, m_Specification.Width, m_Specification.Height);
#else
            m_ColorAttachments.resize(m_ColorAttachmentSpecs.size());
            m_ColorAttachmentImages.resize(m_ColorAttachmentSpecs.size());

            for (size_t i = 0; i < m_Specification.ExistingImages.size(); i++)
            {
                m_ColorAttachments[i] = m_Specification.ExistingImages[i]->GetRendererID();
                m_ColorAttachmentImages[i] = m_Specification.ExistingImages[i];
            }

            size_t existCount = m_Specification.ExistingImages.size();
            size_t count = m_ColorAttachments.size() - existCount;
            RendererID* startPtr = m_ColorAttachments.data() + sizeof(RendererID) * existCount;
            Utils::CreateTexture(multisampled, startPtr, count);

            for (size_t i = 0; i < m_ColorAttachmentSpecs.size(); i++)
            {
                TextureSpecification spec;
                spec.Samples = m_Specification.Samples;
                spec.Format = m_ColorAttachmentSpecs[i].TextureFormat;
                spec.Width = m_Specification.Width;
                spec.Height = m_Specification.Height;
                if (m_ColorAttachmentSpecs[i].TextureFormat == ImageFormat::RED32I)
                {
                    spec.MinFilter = ImageParameter::NEAREST;
                    spec.MagFilter = ImageParameter::NEAREST;
                }
                spec.WrapR = ImageParameter::CLAMP_TO_BORDER;
                spec.WrapS = ImageParameter::CLAMP_TO_BORDER;
                spec.WrapT = ImageParameter::CLAMP_TO_BORDER;

                Utils::BindTexture(multisampled, m_ColorAttachments[i]);
                switch (m_ColorAttachmentSpecs[i].TextureFormat)
                {
                    case ImageFormat::RG16F:
                        Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RG16F, GL_RG, GL_FLOAT, m_Specification.Width, m_Specification.Height, i);
                        break;
                    case ImageFormat::RGBA8:
                        Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGBA8, GL_RGB, GL_UNSIGNED_BYTE, m_Specification.Width, m_Specification.Height, i);
                        break;
                    case ImageFormat::RGB16F:
                        Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGB16F, GL_RGB, GL_HALF_FLOAT, m_Specification.Width, m_Specification.Height, i);
                        break;
                    case ImageFormat::RGBA32F:
                        Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGBA32F, GL_RGBA, GL_FLOAT, m_Specification.Width, m_Specification.Height, i);
                        break;
                    case ImageFormat::RED32I:
                        Utils::AttachColorTexture(m_ColorAttachments[i], spec, i);
                        break;
                    default:
                        break;
                }

                m_ColorAttachmentImages[i] = Texture2D::Create(m_ColorAttachments[i], spec);
            }
#endif
        }

        if (m_DepthAttachmentSpec.TextureFormat != ImageFormat::None)
        {
            if (m_Specification.DepthRenderbuffer)
            {
                glGenRenderbuffers(1, &m_DepthAttachment); GCE;
                glBindRenderbuffer(GL_RENDERBUFFER, m_DepthAttachment); GCE;
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_Specification.Width, m_Specification.Height); GCE;
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthAttachment); GCE;
            } else {
                Utils::CreateTexture(multisampled, &m_DepthAttachment, 1);
                Utils::BindTexture(multisampled, m_DepthAttachment);
                switch (m_DepthAttachmentSpec.TextureFormat)
                {
                    case ImageFormat::DEPTH24STENCIL8:
                        Utils::AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
                        break;
                    default:
                        break;
                }

                TextureSpecification spec;
                spec.Samples = m_Specification.Samples;
                spec.Format = m_DepthAttachmentSpec.TextureFormat;
                spec.Width = m_Specification.Width;
                spec.Height = m_Specification.Height;
                spec.WrapR = ImageParameter::CLAMP_TO_BORDER;
                spec.WrapS = ImageParameter::CLAMP_TO_BORDER;
                spec.WrapT = ImageParameter::CLAMP_TO_BORDER;
                m_DepthAttachmentImage = Texture2D::Create(m_DepthAttachment, spec);
            }
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

    // TODO: fix issue
    void OpenGLFramebuffer::CreateTextures(int samples, std::vector<FramebufferTextureSpecification> attachmentSpecs, uint32_t width, uint32_t height)
    {
        uint32_t count = attachmentSpecs.size();
        bool multisampled = samples > 1;
        GLenum target = multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

        m_ColorAttachments.resize(count);
        m_ColorAttachmentImages.resize(count);

        for (size_t i = 0; i < count; i++)
        {
            TextureSpecification spec;
            spec.Format = attachmentSpecs[i].TextureFormat;
            spec.Samples = samples;
            spec.Width = width;
            spec.Height = height;
            spec.WrapR = ImageParameter::CLAMP_TO_BORDER;
            spec.WrapS = ImageParameter::CLAMP_TO_BORDER;
            spec.WrapT = ImageParameter::CLAMP_TO_BORDER;

            Ref<Texture2D> texture = Texture2D::Create(spec);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, target, texture->GetRendererID(), 0); GCE;

            m_ColorAttachments.push_back(texture->GetRendererID());
            m_ColorAttachmentImages.push_back(texture);
        }
    }
}