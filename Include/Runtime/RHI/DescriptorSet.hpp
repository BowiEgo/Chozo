#pragma once

#include <Runtime/RHI/GraphicsBuffer.hpp>
#include <Runtime/RHI/RHITypes.hpp>
#include <Runtime/RHI/SetLayout.hpp>
#include <Runtime/RHI/Texture.hpp>

namespace CZ {

struct DescriptorBinding {
    uint32_t m_Binding;
    UniformType m_Type;
    GraphicsBuffer m_Buffer; // (Type == UniformBuffer or StorageBuffer)
    Texture m_Texture;
    Sampler m_Sampler; // Optional, can be null if texture has its own sampler
    ImageLayout m_ImageLayout = ImageLayout::ShaderReadOnlyOptimal;

    DescriptorBinding(uint32_t binding = 0, GraphicsBuffer buffer = GraphicsBuffer(),
                      UniformType type = UniformType::UniformBuffer)
        : m_Binding(binding), m_Type(type), m_Buffer(buffer) {}

    DescriptorBinding(uint32_t binding = 0, Texture texture = Texture(),
                      Sampler sampler    = Sampler(),
                      ImageLayout layout = ImageLayout::ShaderReadOnlyOptimal,
                      UniformType type   = UniformType::CombinedImageSampler)
        : m_Binding(binding), m_Type(type), m_Texture(texture), m_Sampler(sampler),
          m_ImageLayout(layout) {}
};

class DescriptorSetObj {
public:
    DescriptorSetObj(SetLayout setLayout, std::vector<DescriptorBinding>& bindings)
        : m_SetLayout(setLayout), m_ResourceBindings(bindings) {}
    virtual ~DescriptorSetObj() = default;

    virtual void* GetRawHandle() const = 0;

protected:
    SetLayout m_SetLayout;
    std::vector<DescriptorBinding> m_ResourceBindings;
};

struct DescriptorSet : Handle<class DescriptorSetObj> {
    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }
};

} // namespace CZ
