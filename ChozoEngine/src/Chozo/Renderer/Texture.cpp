#include "Texture.h"

#include "RenderCommand.h"
#include "Chozo/Renderer/Backend/OpenGL/OpenGLTexture.h"

namespace Chozo
{

    //==============================================================================
    // Texture
    void Texture::ConvertColorSpace(const Buffer &buffer, const ColorSpace colorSpace, const uint32_t width, const uint32_t height, const uint32_t channelCount) const
    {
        if (colorSpace == ColorSpace::SRGB) {
            const auto data = (uint8_t*)buffer.Data;
            const size_t numPixels = width * height;

            for (size_t i = 0; i < numPixels; ++i) {
                const uint8_t r = data[i * channelCount + 0];
                const uint8_t g = data[i * channelCount + 1];
                const uint8_t b = data[i * channelCount + 2];

                float rf = r / 255.0f;
                float gf = g / 255.0f;
                float bf = b / 255.0f;

                // sRGB to Linear conversion for each color channel
                rf = Color::SRGBToLinear(rf);
                gf = Color::SRGBToLinear(gf);
                bf = Color::SRGBToLinear(bf);

                data[i * channelCount + 0] = static_cast<uint8_t>(std::min(1.0f, std::max(0.0f, rf)) * 255);
                data[i * channelCount + 1] = static_cast<uint8_t>(std::min(1.0f, std::max(0.0f, gf)) * 255);
                data[i * channelCount + 2] = static_cast<uint8_t>(std::min(1.0f, std::max(0.0f, bf)) * 255);
            }
        }
    }

	//==============================================================================
	// Texture2D
    TextureType Texture2D::s_Type = TextureType::Texture2D;

    Ref<Texture2D> Texture2D::Create(const Texture2DSpecification &spec)
    {
        switch (RenderCommand::GetType())
        {
            case RenderAPI::Type::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RenderAPI::Type::OpenGL:   return Ref<OpenGLTexture2D>::Create(spec);
        }

        CZ_CORE_ASSERT(false, "Unknown RenderAPI!");
        return nullptr;
    }

    Ref<Texture2D> Texture2D::Create(const Ref<Texture2D> other)
    {
        switch (RenderCommand::GetType())
        {
            case RenderAPI::Type::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RenderAPI::Type::OpenGL:   return Ref<OpenGLTexture2D>::Create(other);
        }

        CZ_CORE_ASSERT(false, "Unknown RenderAPI!");
        return nullptr;
    }

    Ref<Texture2D> Texture2D::Create(const std::string &path, const Texture2DSpecification& spec)
    {
        switch (RenderCommand::GetType())
        {
            case RenderAPI::Type::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RenderAPI::Type::OpenGL:   return Ref<OpenGLTexture2D>::Create(path, spec);
        }

        CZ_CORE_ASSERT(false, "Unknown RenderAPI!");
        return nullptr;
    }

    Ref<Texture2D> Texture2D::Create(const RendererID &id, const Texture2DSpecification &spec)
    {
        switch (RenderCommand::GetType())
        {
            case RenderAPI::Type::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RenderAPI::Type::OpenGL:   return Ref<OpenGLTexture2D>::Create(id, spec);
        }

        CZ_CORE_ASSERT(false, "Unknown RenderAPI!");
        return nullptr;
    }

    Ref<Texture2D> Texture2D::Create(const Buffer& imageBuffer, const Texture2DSpecification &spec)
    {
        switch (RenderCommand::GetType())
        {
            case RenderAPI::Type::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RenderAPI::Type::OpenGL:   return Ref<OpenGLTexture2D>::Create(imageBuffer, spec);
        }

        CZ_CORE_ASSERT(false, "Unknown RenderAPI!");
        return nullptr;
    }

	//==============================================================================
	// TextureCube
    TextureType TextureCube::s_Type = TextureType::TextureCube;

    Ref<TextureCube> TextureCube::Create(const TextureCubeSpecification& spec)
    {
        switch (RenderCommand::GetType())
        {
            case RenderAPI::Type::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RenderAPI::Type::OpenGL:   return Ref<OpenGLTextureCube>::Create(spec);
        }

        CZ_CORE_ASSERT(false, "Unknown RenderAPI!");
        return nullptr;
    }
}
