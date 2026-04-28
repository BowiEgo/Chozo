#pragma once

#include "Asset.h"
#include "RHIContext.h"
#include "RHIShader.h"
#include "Ref.h"
#include "RenderCoreExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogShader, Info);

struct FShaderSpecification {
    std::string Name;
    std::string VirtualPath;
    std::vector<EShaderStage> Stages;
    std::string EntryPoint = "main";
    FShaderDefinitions Definitions; // Macros for permutations, e.g., {"USE_ALBEDO": "1"}

    FShaderSpecification(const std::string name, const std::string path,
                         const std::vector<EShaderStage> stages, const std::string entry,
                         const FShaderDefinitions& defs = {})
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

class RENDER_CORE_API CShader : public IAsset {
public:
    CShader(const FShaderSpecification& spec,
            const std::unordered_map<EShaderStage, FShaderCompilerOutput>& outputs);
    virtual ~CShader() = default;

    virtual const std::string GetName() const override { return m_Spec.Name; }
    virtual const EAssetType GetType() const override { return EAssetType::Shader; }

    TRef<IRHISetLayout> GetSetLayout(uint32_t set) {
        if (m_ShaderResources.empty()) {
            CreateRHIDeviceResources();
        }
        auto it = m_SetLayouts.find(set);
        return it != m_SetLayouts.end() ? it->second : nullptr;
    }
    const std::unordered_map<uint32_t, TRef<IRHISetLayout>>& GetAllSetLayouts() const {
        return m_SetLayouts;
    }
    const VertexBufferLayout GetVertexLayout();
    const std::vector<FPushConstantRange>& GetPushConstantRanges();
    const std::vector<TRef<IRHIShader>>& GetShaderResources() {
        if (m_ShaderResources.empty()) {
            CreateRHIDeviceResources();
        }
        return m_ShaderResources;
    }

    void CreateRHIDeviceResources();

private:
    void BuildLayouts();

private:
    const FShaderSpecification m_Spec;
    std::unordered_map<EShaderStage, FShaderCompilerOutput> m_Datas;

    mutable std::vector<TRef<IRHIShader>> m_ShaderResources;
    mutable std::vector<FPushConstantRange> m_PushConstantRanges;

    mutable std::unordered_map<uint32_t, TRef<IRHISetLayout>> m_SetLayouts;

    TRef<IRHIDescriptorSet> m_DescriptorSet;
    mutable bool m_bDescriptorSetDirty = true;
};
