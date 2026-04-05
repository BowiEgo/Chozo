#pragma once

#include "RHIExport.h"
#include "RHITypes.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHIShader, Info);

using FShaderDefinitions = std::map<std::string, std::string>;

struct FShaderSpecification {
    std::string Name;
    std::string VirtualPath;
    EShaderStage Stage;
    std::string EntryPoint = "main";
    FShaderDefinitions Definitions; // Macros for permutations, e.g., {"USE_ALBEDO": "1"}

    FShaderSpecification(const std::string name, const std::string path, const EShaderStage stage,
                         const std::string entry, const FShaderDefinitions& defs = {})
        : Name(name), VirtualPath(path), Stage(stage), EntryPoint(entry), Definitions(defs) {}

    size_t GenHash() const {
        size_t h = std::hash<std::string>{}(VirtualPath);

        HashCombine(h, static_cast<size_t>(Stage));
        HashCombine(h, std::hash<std::string>{}(EntryPoint));

        for (const auto& [key, value] : Definitions) {
            HashCombine(h, std::hash<std::string>{}(key));
            HashCombine(h, std::hash<std::string>{}(value));
        }

        return h;
    }
};

class RHI_API IRHIShader : public FRefCounted {
public:
    IRHIShader(const FShaderSpecification& spec, const FShaderReflection reflection);
    virtual ~IRHIShader();

    const EShaderStage GetStage() const { return m_Spec.Stage; }
    const std::string& GetName() const { return m_Spec.Name; }
    const std::string& GetEntryPoint() const { return m_Spec.EntryPoint; }
    const FShaderReflection GetReflection() const { return m_Reflection; }

    static TRef<IRHIShader> Create(const FShaderSpecification& spec,
                                   const std::vector<uint32_t>* binary);

protected:
    FShaderSpecification m_Spec;
    FShaderReflection m_Reflection;
};