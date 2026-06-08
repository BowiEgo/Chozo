#include <Runtime/RenderCore/Shader.hpp>

#include "ShaderUtils.hpp"

#include <Core/Log/LogMacros.hpp>
#include <Runtime/RHI/RHIAPI.hpp>

namespace CZ {

template <> void Handle<ShaderObj>::Destroy() {
    if (m_Obj) {
        for (auto& shaderRes : m_Obj->m_ShaderResources) {
            shaderRes.Destroy();
        }
        m_Obj->m_ShaderResources.clear();
    }
}

SetLayout ShaderObj::GetSetLayout(uint32_t set) {
    if (m_ShaderResources.empty()) {
        CreateShaderResources();
    }
    auto it = m_SetLayouts.find(set);
    return it != m_SetLayouts.end() ? it->second : SetLayout();
}

const std::vector<PushConstantRange>& ShaderObj::GetPushConstantRanges() {
    if (!m_PushConstantRanges.empty()) return m_PushConstantRanges;

    std::unordered_map<uint32_t, PushConstantRange> tempMap;
    for (const auto& shaderRes : GetShaderResources()) {
        ShaderStage stage      = shaderRes->GetStage();
        const auto& reflection = GetReflection();
        for (const auto& uniform : reflection.Uniforms) {
            if (uniform.Type == UniformType::PushConstant) {
                uint32_t offset = uniform.Offset;
                uint32_t size   = uniform.Size;
                auto it         = tempMap.find(offset);
                if (it != tempMap.end()) {
                    it->second.StageFlags |= stage;
                    if (size > it->second.Size) it->second.Size = size;
                } else {
                    PushConstantRange range;
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
    std::sort(
        m_PushConstantRanges.begin(), m_PushConstantRanges.end(),
        [](const PushConstantRange& a, const PushConstantRange& b) { return a.Offset < b.Offset; });

    return m_PushConstantRanges;
}

void ShaderObj::CreateShaderResources() {
    m_ShaderResources.reserve(m_Datas.size());

    for (auto& [stage, output] : m_Datas) {
        ShaderResSpecification spec;
        spec.Name       = m_Spec.Name + "_" + ShaderUtils::StageToString(stage);
        spec.Stage      = stage;
        spec.EntryPoint = output.EntryPoint;

        auto shaderRes = RHIAPI::Get()->CreateShaderRes(spec, &output.Binary);

        m_ShaderResources.push_back(shaderRes);
    }

    // BuildLayouts();
}
} // namespace CZ