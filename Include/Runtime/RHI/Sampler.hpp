#pragma once

#include <Core/Header/Handle.hpp>
#include <Core/Memory/Memory.hpp>
#include <Runtime/RHI/RHITypes.hpp>

namespace CZ {

struct SamplerSpecification {
    Filter MinFilter         = Filter::Linear;
    Filter MagFilter         = Filter::Linear;
    AddressMode AddressModeU = AddressMode::Repeat;
    AddressMode AddressModeV = AddressMode::Repeat;
    AddressMode AddressModeW = AddressMode::Repeat;
    MipmapMode MipmapMode    = MipmapMode::Linear;
    float LodBias            = 0.0f;
    float MaxAnisotropy      = 1.0f;
    bool bEnableAnisotropy   = false;

    static SamplerSpecification LinearClamp() {
        SamplerSpecification spec;
        spec.AddressModeU = AddressMode::ClampToEdge;
        spec.AddressModeV = AddressMode::ClampToEdge;
        spec.AddressModeW = AddressMode::ClampToEdge;
        return spec;
    }

    static SamplerSpecification LinearClampToBorder() {
        SamplerSpecification spec;
        spec.AddressModeU = AddressMode::ClampToBorder;
        spec.AddressModeV = AddressMode::ClampToBorder;
        spec.AddressModeW = AddressMode::ClampToBorder;
        spec.MagFilter    = Filter::Linear;
        spec.MinFilter    = Filter::Linear;
        return spec;
    }

    static SamplerSpecification Repeat() {
        SamplerSpecification spec;
        spec.AddressModeU = AddressMode::Repeat;
        spec.AddressModeV = AddressMode::Repeat;
        spec.AddressModeW = AddressMode::Repeat;
        spec.MagFilter    = Filter::Linear;
        spec.MinFilter    = Filter::Linear;
        return spec;
    }

    static SamplerSpecification PointClamp() {
        SamplerSpecification spec;
        spec.MinFilter    = Filter::Nearest;
        spec.MagFilter    = Filter::Nearest;
        spec.AddressModeU = AddressMode::ClampToEdge;
        spec.AddressModeV = AddressMode::ClampToEdge;
        spec.AddressModeW = AddressMode::ClampToEdge;
        return spec;
    }

    static SamplerSpecification PointClampToBorder() {
        SamplerSpecification spec;
        spec.MinFilter    = Filter::Nearest;
        spec.MagFilter    = Filter::Nearest;
        spec.AddressModeU = AddressMode::ClampToBorder;
        spec.AddressModeV = AddressMode::ClampToBorder;
        spec.AddressModeW = AddressMode::ClampToBorder;
        return spec;
    }

    bool operator==(const SamplerSpecification& other) const {
        return MinFilter == other.MinFilter && MagFilter == other.MagFilter &&
               AddressModeU == other.AddressModeU && AddressModeV == other.AddressModeV &&
               AddressModeW == other.AddressModeW && MipmapMode == other.MipmapMode &&
               LodBias == other.LodBias && MaxAnisotropy == other.MaxAnisotropy &&
               bEnableAnisotropy == other.bEnableAnisotropy;
    }
};

class SamplerObj;

struct Sampler : Handle<class SamplerObj> {
    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }
};
} // namespace CZ

namespace std {
template <> struct hash<CZ::SamplerSpecification> {
    size_t operator()(const CZ::SamplerSpecification& spec) const {
        size_t h = 0;
        HashCombine(h, static_cast<size_t>(spec.MinFilter));
        HashCombine(h, static_cast<size_t>(spec.MagFilter));
        HashCombine(h, static_cast<size_t>(spec.AddressModeU));
        HashCombine(h, static_cast<size_t>(spec.AddressModeV));
        HashCombine(h, static_cast<size_t>(spec.AddressModeW));
        HashCombine(h, static_cast<size_t>(spec.MipmapMode));
        HashCombine(h, std::hash<float>{}(spec.LodBias));
        HashCombine(h, std::hash<float>{}(spec.MaxAnisotropy));
        HashCombine(h, std::hash<bool>{}(spec.bEnableAnisotropy));
        return h;
    }
};
} // namespace std