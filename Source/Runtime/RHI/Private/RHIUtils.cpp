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

bool IsSRGBFormat(EPixelFormat format) {
    return format == EPixelFormat::RGBA8_SRGB || format == EPixelFormat::BGRA8_SRGB ||
           format == EPixelFormat::BC1_RGB_SRGB || format == EPixelFormat::BC3_SRGB ||
           format == EPixelFormat::BC7_SRGB;
}

bool IsDepthFormat(EPixelFormat format) {
    return format == EPixelFormat::D16_UNORM || format == EPixelFormat::D32F ||
           format == EPixelFormat::D24S8 || format == EPixelFormat::D32F_S8;
}

bool HasStencilComponent(EPixelFormat format) {
    return format == EPixelFormat::D24S8 || format == EPixelFormat::D32F_S8;
}

EImageAspect GetDefaultAspectMask(EPixelFormat format) {
    if (IsDepthFormat(format)) {
        EImageAspect aspect = EImageAspect::Depth;

        if (HasStencilComponent(format)) {
            aspect = aspect | EImageAspect::Stencil;
        }
        return aspect;
    }

    return EImageAspect::Color;
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

EShaderDataType ToShaderDataFormat(EPixelFormat format) {
    switch (format) {
        // Single-channel
        case EPixelFormat::R8_UNORM:
        case EPixelFormat::R16_UNORM:
        case EPixelFormat::R16F:
        case EPixelFormat::R32F:
        case EPixelFormat::D16_UNORM:
        case EPixelFormat::D24S8:
        case EPixelFormat::D32F: return EShaderDataType::Float;

        // Dual-channel
        case EPixelFormat::RG8_UNORM:
        case EPixelFormat::RG16F:
        case EPixelFormat::RG32F: return EShaderDataType::Float2;

        case EPixelFormat::RGB9E5:
        case EPixelFormat::R11G11B10F: return EShaderDataType::Float3;

        // 4-channel RGBA/BGRA
        case EPixelFormat::RGBA8_UNORM:
        case EPixelFormat::RGBA8_SRGB:
        case EPixelFormat::BGRA8_UNORM:
        case EPixelFormat::BGRA8_SRGB:
        case EPixelFormat::RGBA16_UNORM:
        case EPixelFormat::RGBA16F:
        case EPixelFormat::RGBA32F: return EShaderDataType::Float4;

        default: return EShaderDataType::None;
    }
}

} // namespace ChozoUtils::RHI