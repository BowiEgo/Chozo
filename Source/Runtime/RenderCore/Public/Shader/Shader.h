#pragma once

#include "GraphicsContext.h"
#include "RHIShader.h"
#include "Ref.h"
#include "RenderCoreExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogShader, Info);

using FShaderDefinitions = std::map<std::string, std::string>;

struct FShaderCreateInfo {
    std::string Name;
    std::string VirtualPath;
    EShaderStage Stage;
    std::string EntryPoint = "main";
    FShaderDefinitions Definitions; // Macros for permutations, e.g., {"USE_ALBEDO": "1"}

    FShaderCreateInfo(const std::string name, const std::string path, const EShaderStage stage,
                      const std::string entry, const FShaderDefinitions& defs = {})
        : Name(name), VirtualPath(path), Stage(stage), EntryPoint(entry), Definitions(defs) {}

    size_t GenHash() const {
        size_t h = std::hash<std::string>{}(VirtualPath);

        auto hashCombine = [](size_t& seed, size_t value) {
            seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        };

        hashCombine(h, static_cast<size_t>(Stage));
        hashCombine(h, std::hash<std::string>{}(EntryPoint));

        for (const auto& [key, value] : Definitions) {
            hashCombine(h, std::hash<std::string>{}(key));
            hashCombine(h, std::hash<std::string>{}(value));
        }

        return h;
    }
};

class RENDER_CORE_API CShader : public FRefCounted {
public:
    CShader(const FShaderCreateInfo& info, const FShaderCompilerOutput& data,
            const WeakRef<IRHIDevice> device);
    virtual ~CShader() = default;

    const FShaderID& GetID() const { return m_ID; }
    const std::string& GetName() const { return m_Info.Name; }
    TRef<IRHIShader> GetRHIShader() {
        if (!m_RHIShader) {
            m_RHIShader = CreateRHIDeviceResource(m_Info, m_Data);
        }
        return m_RHIShader;
    }

    TRef<IRHIShader> CreateRHIDeviceResource(const FShaderCreateInfo& info,
                                             const FShaderCompilerOutput& data);

protected:
    FShaderID m_ID{};

    FShaderCreateInfo m_Info;
    FShaderReflection m_Reflection;
    FShaderCompilerOutput m_Data;
    WeakRef<IRHIDevice> m_Device;
    TRef<IRHIShader> m_RHIShader;
};
