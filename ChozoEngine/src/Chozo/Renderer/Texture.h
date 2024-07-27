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
        Texture2DParameter minFilter = Texture2DParameter::LINEAR;
        Texture2DParameter magFilter = Texture2DParameter::NEAREST;
        Texture2DParameter wrapS = Texture2DParameter::REPEAT;
        Texture2DParameter wrapT = Texture2DParameter::REPEAT;
    };

    class Texture2D : public Texture
    {
    public:
        static Ref<Texture2D> Create(const std::string& path, const Texture2DSpecification& spec = Texture2DSpecification());
    };
}