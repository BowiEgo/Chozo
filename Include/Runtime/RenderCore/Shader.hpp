#pragma once

#include <Core/Header/Handle.hpp>
#include <Core/Memory/Memory.hpp>
#include <Runtime/RHI/RHITypes.hpp>
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
                        const std::vector<ShaderStage> stages, const std::string entry,
                        const ShaderDefinitions& defs = {})
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

struct ShaderObj {
    ShaderSpecification Spec;
    std::unordered_map<ShaderStage, ShaderCompilerOutput> Datas;
    mutable std::vector<ShaderRes> ShaderResources;

    // std::string m_Name;
    // std::filesystem::path m_VirtualPath;
    // std::vector<ShaderStage> m_Stages;
    // std::string m_EntryPoint;
    // ShaderDefinitions m_Definitions;

    // std::unordered_map<ShaderStage, std::vector<uint32_t>> m_SPIRVBlobs;

    // std::unordered_map<ShaderStage, ShaderReflection> m_Reflections;

    ShaderObj(const ShaderSpecification& spec,
              const std::unordered_map<ShaderStage, ShaderCompilerOutput>& compiledOutputs)
        : Spec(spec), Datas(compiledOutputs) {}

    ShaderSpecification GetSpec() const { return Spec; }

    const std::vector<ShaderRes>& GetShaderResources() {
        if (ShaderResources.empty()) CreateShaderResources();
        return ShaderResources;
    }

    void CreateShaderResources();

    bool LoadAndCompile();
};

class Shader : public Asset<struct ShaderObj> {
public:
    static Shader
        Create(const ShaderSpecification& spec,
               const std::unordered_map<ShaderStage, ShaderCompilerOutput>& compiledOutputs) {
        auto obj = CZ_NEW(MEMORY_USAGE_ASSET, ShaderObj, spec, compiledOutputs);

        return Shader(obj);
    }

    Shader() = default;
    explicit Shader(ShaderObj* obj) : Asset<ShaderObj>(obj) {}

    AssetType GetType() const override { return AssetType::Shader; }

    const std::string GetName() const override { return (*this)->GetSpec().Name; }
};

} // namespace CZ
