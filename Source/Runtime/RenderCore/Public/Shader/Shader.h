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

    const std::string& GetName() const { return m_Spec.Name; }

    std::vector<TRef<IRHIShader>> GetShaderResources() {
        if (m_ShaderResources.empty()) {
            CreateRHIDeviceResources();
        }
        return m_ShaderResources;
    }

    void CreateRHIDeviceResources();

protected:
    const FShaderSpecification m_Spec;
    std::unordered_map<EShaderStage, FShaderCompilerOutput> m_Datas;

    std::vector<TRef<IRHIShader>> m_ShaderResources;
};
