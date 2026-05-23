#pragma once

#include <Core/Header/Handle.hpp>
#include <Runtime/RHI/RHITypes.hpp>

namespace CZ {

struct SetLayoutDescription {
    std::vector<ShaderResourceBinding> Bindings;

    void AddBinding(const uint32_t binding, const UniformType type, const uint32_t descriptorCount,
                    const ShaderStage stage) {
        Bindings.push_back(ShaderResourceBinding(binding, type, descriptorCount, stage));
    }

    size_t GetHash() const {
        size_t h = 0;
        for (const auto& b : Bindings) {
            HashCombine(h, std::hash<uint32_t>{}(b.Binding));
            HashCombine(h, std::hash<uint32_t>{}(static_cast<uint32_t>(b.Type)));
            HashCombine(h, std::hash<uint32_t>{}(static_cast<uint32_t>(b.DescriptorCount)));
            HashCombine(h, std::hash<uint32_t>{}(static_cast<uint32_t>(b.StageFlags)));
        }
        return h;
    }

    bool operator==(const SetLayoutDescription& other) const { return Bindings == other.Bindings; }
};

class SetLayoutObj {
public:
    SetLayoutObj() = default;

    virtual ~SetLayoutObj() = default;
};

struct SetLayout : Handle<class SetLayoutObj> {
    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }
};

} // namespace CZ
