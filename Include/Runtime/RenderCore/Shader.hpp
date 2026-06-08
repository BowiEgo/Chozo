#pragma once

#include <Core/Header/Handle.hpp>
#include <Core/Memory/Memory.hpp>
#include <Runtime/RHI/RHITypes.hpp>
#include <Runtime/RHI/SetLayout.hpp>
#include <Runtime/RHI/ShaderRes.hpp>
#include <Runtime/RenderCore/Asset.hpp>

namespace CZ {

struct ShaderSpecification {
    std::string Name;
    std::string VirtualPath;
    std::vector<ShaderStage> Stages;
    std::string EntryPoint = "main";
    ShaderDefinitions Definitions; // Macros for permutations, e.g., {"USE_ALBEDO": "1"}

    ShaderSpecification(const std::string name, const std::string path,
                        const std::vector<ShaderStage> stages = { ShaderStage::Vertex,
                                                                  ShaderStage::Fragment },
                        const std::string entry = "main", const ShaderDefinitions& defs = {})
        : Name(name), VirtualPath(path), Stages(stages), EntryPoint(entry), Definitions(defs) {}

    size_t GenHash() const {
        size_t h = std::hash<std::string>{}(VirtualPath);

        for (const auto& stage : Stages) {
            HashCombine(h, static_cast<size_t>(stage));
        }

        HashCombine(h, std::hash<std::string>{}(EntryPoint));

        for (const auto& [key, value] : Definitions) {
            HashCombine(h, std::hash<std::string>{}(key));
            HashCombine(h, std::hash<std::string>{}(value));
        }

        return h;
    }
};

class ShaderObj {
    friend class Handle<ShaderObj>;

public:
    ShaderObj(const ShaderSpecification& spec,
              const std::unordered_map<ShaderStage, ShaderCompilerOutput>& compiledOutputs)
        : m_Spec(spec), m_Datas(compiledOutputs) {}

    ShaderSpecification GetSpec() const { return m_Spec; }

    SetLayout GetSetLayout(uint32_t set);
    const std::unordered_map<uint32_t, SetLayout>& GetAllSetLayouts() const { return m_SetLayouts; }
    const VertexBufferLayout GetVertexLayout();
    const std::vector<PushConstantRange>& GetPushConstantRanges();

    const std::vector<ShaderRes>& GetShaderResources() {
        if (m_ShaderResources.empty()) CreateShaderResources();
        return m_ShaderResources;
    }

    ShaderReflection GetReflection() const {
        // Assuming all stages share the same reflection for simplicity
        if (!m_Datas.empty()) {
            return m_Datas.begin()->second.Reflection;
        }
        return ShaderReflection{};
    }

    void CreateShaderResources();

private:
    ShaderSpecification m_Spec;
    std::unordered_map<ShaderStage, ShaderCompilerOutput> m_Datas;
    mutable std::vector<ShaderRes> m_ShaderResources;
    mutable std::vector<PushConstantRange> m_PushConstantRanges;
    mutable std::unordered_map<uint32_t, SetLayout> m_SetLayouts;

    // std::string m_Name;
    // std::filesystem::path m_VirtualPath;
    // std::vector<ShaderStage> m_Stages;
    // std::string m_EntryPoint;
    // ShaderDefinitions m_Definitions;

    // std::unordered_map<ShaderStage, std::vector<uint32_t>> m_SPIRVBlobs;

    // std::unordered_map<ShaderStage, ShaderReflection> m_Reflections;
};

class Shader : public Asset<class ShaderObj> {
public:
    static Shader
        Create(const ShaderSpecification& spec,
               const std::unordered_map<ShaderStage, ShaderCompilerOutput>& compiledOutputs) {
        auto obj = CZ_NEW(MEMORY_USAGE_ASSET, ShaderObj, spec, compiledOutputs);

        return Shader(obj);
    }

    Shader() = default;
    explicit Shader(ShaderObj* obj) : Asset<ShaderObj>(obj) {}
    virtual ~Shader() = default;

    AssetType GetType() const override { return AssetType::Shader; }

    const std::string GetName() const override { return (*this)->GetSpec().Name; }
};

template <> struct AssetTraits<ShaderObj> {
    using AssetClass = Shader;
};

} // namespace CZ
