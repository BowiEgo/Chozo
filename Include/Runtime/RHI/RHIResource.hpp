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

/// Base for RHI objects that need a stable identity (descriptor cache keys, ImGui texture ids).
/// It deliberately carries no type tag: `ResourceType` describes descriptor bindings, not objects.
class RHIResource {
public:
    RHIResource() : m_ID(UUID::Generate()) {}
    virtual ~RHIResource() = default;

    UUID GetID() const { return m_ID; }

protected:
    UUID m_ID;
};

} // namespace CZ
