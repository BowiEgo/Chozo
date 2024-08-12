#pragma once

#include "czpch.h"
#include "Chozo/Renderer/RendererTypes.h"

namespace Chozo {

    class Texture
    {
    public:
        virtual ~Texture() = default;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
		virtual RendererID GetRendererID() const = 0;

        virtual void Bind(uint32_t slot = 0) const = 0;
    };

    enum class ImageFormat
	{
		None = 0,
		RED8UN,
		RED8UI,
		RED16UI,
		RED32UI,
		RED32F,
		RG8,
		RG16F,
		RG32F,
		RGB,
		RGBA,
		RGBA16F,
		RGBA32F,

		B10R11G11UF,

		SRGB,

		DEPTH32FSTENCIL8UINT,
		DEPTH32F,
		DEPTH24STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8,
	};

    enum class ImageParameter
    {
        None = 0,
        LINEAR,
        NEAREST,
        REPEAT,
        MIRRORED_REPEAT,
        CLAMP_TO_EDGE,
        CLAMP_TO_BORDER
    };

    struct TextureSpecification
    {
		ImageFormat Format = ImageFormat::RGBA;

        uint32_t Width = 1, Height = 1;
        ImageParameter MinFilter = ImageParameter::LINEAR;
        ImageParameter MagFilter = ImageParameter::NEAREST;
        ImageParameter WrapS = ImageParameter::REPEAT;
        ImageParameter WrapT = ImageParameter::REPEAT;
    };

    class Texture2D : public Texture
    {
    public:
        static Ref<Texture2D> Create(const TextureSpecification& spec = TextureSpecification());
        static Ref<Texture2D> Create(const std::string& path, const TextureSpecification& spec = TextureSpecification());
		// static Ref<Texture2D> Create(const TextureSpecification& spec, Buffer imageData);

		virtual void SetData(const void* data, const uint32_t size) = 0;
    };

    class TextureCube : public Texture
    {
    public:
        static Ref<TextureCube> Create(const TextureSpecification& spec = TextureSpecification());
    };
}