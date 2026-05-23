#include <Runtime/RenderCore/Shader.hpp>

#include "ShaderUtils.hpp"

#include <Core/Log/LogMacros.hpp>
#include <Runtime/RHI/RHIAPI.hpp>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogViewport, Info);

DEFINE_HANDLE_DESTROY(ShaderObj)

bool ShaderObj::LoadAndCompile() { return true; }

void ShaderObj::CreateShaderResources() {
    ShaderResources.reserve(Datas.size());

    for (auto& [stage, output] : Datas) {
        ShaderResSpecification spec;
        spec.Name       = Spec.Name + "_" + ShaderUtils::StageToString(stage);
        spec.Stage      = stage;
        spec.EntryPoint = Spec.EntryPoint;

        auto shaderRes = RHIAPI::Get()->CreateShaderRes(spec, &output.Binary);

        ShaderResources.push_back(shaderRes);
    }

    // BuildLayouts();
}
} // namespace CZ