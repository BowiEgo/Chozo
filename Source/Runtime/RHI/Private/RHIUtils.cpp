#include "RHIUtils.h"

DEFINE_LOG_CATEGORY(LogRHIUtils);

static FRHIShaderResourceBinding ToRHIBinding(const FUniformSpecification& uniform,
                                              EShaderStage stage) {
    FRHIShaderResourceBinding binding;
    binding.Binding         = uniform.Binding;
    binding.DescriptorCount = uniform.ArraySize;
    binding.Type            = uniform.Type;
    binding.StageFlags      = stage;
    return binding;
}

namespace ChozoUtils::RHI {

bool IsDepthFormat(EPixelFormat format) {
    return format == EPixelFormat::D32_SFLOAT || format == EPixelFormat::D24_UNORM_S8_UINT ||
           format == EPixelFormat::D16_UNORM;
}

FRHIPipelineLayoutDescription
    GeneratePipelineLayoutDesc(const std::vector<TRef<IRHIShader>>& RHIShaders) {
    FRHIPipelineLayoutDescription pipelineDesc;

    for (auto RHIShader : RHIShaders) {
        const auto& reflection = RHIShader->GetReflection();
        const auto stage       = RHIShader->GetStage();

        CZ_LOG(LogRHIUtils, Trace, "{}", reflection.ToString());

        for (const auto& uniform : reflection.Uniforms) {
            if (uniform.Type == EUniformType::PushConstant) {
                pipelineDesc.PushConstantRanges.push_back({ stage, 0, uniform.Size });
                continue;
            }

            auto binding  = ToRHIBinding(uniform, stage);
            auto& setDesc = pipelineDesc.SetLayouts[uniform.Set];

            // Check if the binding already exists (handles the case where multiple shader stages
            // share the same binding)
            auto it = std::find_if(setDesc.Bindings.begin(), setDesc.Bindings.end(),
                                   [&](const auto& b) { return b.Binding == binding.Binding; });

            if (it != setDesc.Bindings.end()) {
                it->StageFlags |= binding.StageFlags;
            } else {
                setDesc.Bindings.push_back(binding);
            }
        }
    }

    for (auto& [set, desc] : pipelineDesc.SetLayouts) {
        std::sort(desc.Bindings.begin(), desc.Bindings.end(),
                  [](const auto& a, const auto& b) { return a.Binding < b.Binding; });
    }
    return pipelineDesc;
}

} // namespace ChozoUtils::RHI