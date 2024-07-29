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

    enum class Texture2DFormat
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

    enum class Texture2DParameter
    {
        None = 0,
        LINEAR,
        NEAREST,
        REPEAT,
        MIRRORED_REPEAT,
        CLAMP_TO_EDGE,
        CLAMP_TO_BORDER
    };

    struct Texture2DSpecification
    {
		Texture2DFormat Format = Texture2DFormat::RGBA;

        uint32_t width = 1, height = 1;
        Texture2DParameter minFilter = Texture2DParameter::LINEAR;
        Texture2DParameter magFilter = Texture2DParameter::NEAREST;
        Texture2DParameter wrapS = Texture2DParameter::REPEAT;
        Texture2DParameter wrapT = Texture2DParameter::REPEAT;
    };

    class Texture2D : public Texture
    {
    public:
        static Ref<Texture2D> Create(const Texture2DSpecification& spec = Texture2DSpecification());
        static Ref<Texture2D> Create(const std::string& path, const Texture2DSpecification& spec = Texture2DSpecification());
		// static Ref<Texture2D> Create(const Texture2DSpecification& spec, Buffer imageData);

		virtual void SetData(const void* data, const uint32_t size) = 0;
    };
}