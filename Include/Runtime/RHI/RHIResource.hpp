#pragma once

#include <Core/Header/Handle.hpp>
#include <Core/Header/UUID.hpp>

namespace CZ {

enum class ResourceType {
    Unknown,
    Texture,
    GraphicsBuffer,
    Sampler,
    Image,
    SetLayout,
    DescriptorSet,
    // ...
};

class RHIResource {
public:
    RHIResource() : m_ID(UUID::Generate()) {}
    ~RHIResource() = default;

    virtual ResourceType GetResourceType() const = 0;

    UUID GetID() const { return m_ID; }

protected:
    UUID m_ID;
};

} // namespace CZ
