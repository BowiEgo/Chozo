#include "Shader.h"

#include "RHIAPI.h"
#include "RHIUtils.h"
#include "ShaderUtils.h"

DEFINE_LOG_CATEGORY(LogShader);

CShader::CShader(const FShaderSpecification& spec,
                 const std::unordered_map<EShaderStage, FShaderCompilerOutput>& outputs)
    : m_Spec(spec), m_Datas(outputs) {
    CZ_LOG(LogShader, Trace, "Creating shader {} ...", m_Spec.Name);
}

const VertexBufferLayout CShader::GetVertexLayout() {
    VertexBufferLayout layout;

    for (const auto& rhiShader : GetShaderResources()) {
        if (rhiShader->GetStage() == EShaderStage::Vertex) {
            auto reflection = rhiShader->GetReflection();

            std::sort(reflection.Attributes.begin(), reflection.Attributes.end(),
                      [](const auto& a, const auto& b) { return a.Location < b.Location; });

            for (const auto& attr : reflection.Attributes) {
                layout.AddElement(attr.Type, attr.Name, attr.Location);
            }
            break;
        }
    }

    // layout.AddElement(EShaderDataType::Float3, "a_Tangent", 3);
    // layout.AddElement(EShaderDataType::Float3, "a_Bitangent", 4);
    return layout;
}

const std::vector<FPushConstantRange>& CShader::GetPushConstantRanges() {
    if (!m_PushConstantRanges.empty()) return m_PushConstantRanges;

    std::unordered_map<uint32_t, FPushConstantRange> tempMap;
    for (const auto& rhiShader : GetShaderResources()) {
        EShaderStage stage     = rhiShader->GetStage();
        const auto& reflection = rhiShader->GetReflection();
        for (const auto& uniform : reflection.Uniforms) {
            if (uniform.Type == EUniformType::PushConstant) {
                uint32_t offset = uniform.Offset;
                uint32_t size   = uniform.Size;
                auto it         = tempMap.find(offset);
                if (it != tempMap.end()) {
                    it->second.StageFlags |= stage;
                    if (size > it->second.Size) it->second.Size = size;
                } else {
                    FPushConstantRange range;
                    range.Offset     = offset;
                    range.Size       = size;
                    range.StageFlags = stage;
                    tempMap[offset]  = range;
                }
            }
        }
    }
    m_PushConstantRanges.reserve(tempMap.size());
    for (auto& pair : tempMap) {
        m_PushConstantRanges.push_back(pair.second);
    }
    std::sort(m_PushConstantRanges.begin(), m_PushConstantRanges.end(),
              [](const FPushConstantRange& a, const FPushConstantRange& b) {
                  return a.Offset < b.Offset;
              });

    return m_PushConstantRanges;
}

void CShader::CreateRHIDeviceResources() {
    m_ShaderResources.reserve(m_Datas.size());

    for (auto& [stage, output] : m_Datas) {
        FRHIShaderSpecification spec;
        spec.Name       = m_Spec.Name + "_" + ChozoUtils::Shader::StageToString(stage);
        spec.Stage      = stage;
        spec.EntryPoint = m_Spec.EntryPoint;

        auto RHIShader = IRHIAPI::CreateShader(spec, &output.Binary, output.Reflection);

        m_ShaderResources.push_back(RHIShader);
    }

    BuildLayouts();
}

void CShader::BuildLayouts() {
    auto rhiShaders = GetShaderResources();

    FRHIPipelineLayoutDescription layoutDesc =
        ChozoUtils::RHI::GeneratePipelineLayoutDesc(rhiShaders);

    for (const auto& [set, setDesc] : layoutDesc.SetLayouts) {
        auto setLayout    = IRHIAPI::GetContext()->GetDevice()->GetOrCreateLayout(setDesc);
        m_SetLayouts[set] = setLayout;
    }
}
