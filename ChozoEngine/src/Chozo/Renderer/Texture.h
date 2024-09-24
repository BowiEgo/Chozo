#pragma once

#include "czpch.h"

#include "RendererTypes.h"
#include "Image.h"

namespace Chozo {

    class Texture : public RefCounted
    {
    public:
        virtual ~Texture() = default;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
		virtual RendererID GetRendererID() const = 0;

        virtual void Bind(uint32_t slot = 0) const = 0;
        virtual void Unbind() const = 0;
    };

    struct TextureSpecification
    {
		ImageFormat Format = ImageFormat::RGBA;

        uint32_t Samples = 1;
        uint32_t Width = 1, Height = 1;
        bool HDR = false;
        bool Mipmap = false;
        ImageParameter MinFilter = ImageParameter::LINEAR;
        ImageParameter MagFilter = ImageParameter::NEAREST;
        ImageParameter WrapR = ImageParameter::CLAMP_TO_BORDER;
        ImageParameter WrapS = ImageParameter::CLAMP_TO_BORDER;
        ImageParameter WrapT = ImageParameter::CLAMP_TO_BORDER;
    };

    struct TextureCubeSpecification
    {
        ImageFormat Format = ImageFormat::RGB16F;

        uint32_t Samples = 1;
        uint32_t Width = 1, Height = 1;
        bool HDR = false;
        bool Mipmap = false;
        ImageParameter MinFilter = ImageParameter::LINEAR;
        ImageParameter MagFilter = ImageParameter::LINEAR;
        ImageParameter WrapR = ImageParameter::CLAMP_TO_EDGE;
        ImageParameter WrapS = ImageParameter::CLAMP_TO_EDGE;
        ImageParameter WrapT = ImageParameter::CLAMP_TO_EDGE;
    };

    class Texture2D : public Texture
    {
    public:
        virtual TextureSpecification GetSpecification() const = 0;

		virtual void SetData(const void* data, const uint32_t size) = 0;

        virtual void Resize(uint32_t width, uint32_t height) = 0;

        static Ref<Texture2D> Create(const TextureSpecification& spec = TextureSpecification());
        static Ref<Texture2D> Create(const std::string& path, const TextureSpecification& spec = TextureSpecification());
        static Ref<Texture2D> Create(const RendererID& id, const TextureSpecification& spec = TextureSpecification());
		// static Ref<Texture2D> Create(const TextureSpecification& spec, Buffer imageData);
    };

    class TextureCube : public Texture
    {
    public:
        static Ref<TextureCube> Create(const TextureCubeSpecification& spec = TextureCubeSpecification());

        virtual void SetData(void* data, uint32_t size) = 0;
    };
}