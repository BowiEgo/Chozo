#include <Runtime/RenderCore/Shader.hpp>

#include "ShaderUtils.hpp"

#include <Core/Log/LogMacros.hpp>
#include <Runtime/RHI/RHIAPI.hpp>

namespace CZ {

template <> void Handle<ShaderObj>::Destroy() {
    if (m_Obj) {
        for (auto& shaderRes : m_Obj->ShaderResources) {
            shaderRes.Destroy();
        }
        m_Obj->ShaderResources.clear();

        Delete(m_Obj);
        m_Obj = nullptr;
    }
}

bool ShaderObj::LoadAndCompile() { return true; }

void ShaderObj::CreateShaderResources() {
    ShaderResources.reserve(Datas.size());

    for (auto& [stage, output] : Datas) {
        ShaderResSpecification spec;
        spec.Name       = Spec.Name + "_" + ShaderUtils::StageToString(stage);
        spec.Stage      = stage;
        spec.EntryPoint = output.EntryPoint;

        auto shaderRes = RHIAPI::Get()->CreateShaderRes(spec, &output.Binary);

        ShaderResources.push_back(shaderRes);
    }

    // BuildLayouts();
}
} // namespace CZ