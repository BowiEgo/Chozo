#include "ShaderUtils.h"
#include "FileUtils.h"
#include "StringUtils.h"

#include <shaderc/shaderc.hpp>

namespace ChozoUtils::Shader {

namespace {

const std::unordered_map<std::string, EShaderStage> s_ShaderExtensionMap = {
#define GENERATE_MAP(ENUM, LOWER, UPPER, SHORT, GLSL, VULKAN) { "." #SHORT, EShaderStage::ENUM },
    FOREACH_SHADER_STAGE(GENERATE_MAP)
#undef GENERATE_MAP
        { ".pixel", EShaderStage::Fragment }
};

} // namespace

EShaderStage StringToStage(std::string_view shaderStage) {
#define GENERATE_IF(ENUM, LOWER, UPPER, SHORT, GLSL, VULKAN)                                       \
    if (shaderStage == #ENUM) return EShaderStage::ENUM;
    FOREACH_SHADER_STAGE(GENERATE_IF)
#undef GENERATE_IF
    return EShaderStage::None;
}

const std::string StageToString(EShaderStage shaderStage, bool bUpper) {
    switch (shaderStage) {
#define GENERATE_CASE(ENUM, LOWER, UPPER, SHORT, GLSL, VULKAN)                                     \
    case EShaderStage::ENUM: return bUpper ? #UPPER : #ENUM;
        FOREACH_SHADER_STAGE(GENERATE_CASE)
#undef GENERATE_CASE
        default: return "Unknown";
    }
}

EShaderStage GetStageFromExtension(const std::string& extension) {
    const std::string ext = ChozoUtils::String::ToLowerCopy(extension);
    if (s_ShaderExtensionMap.find(ext) == s_ShaderExtensionMap.end()) return EShaderStage::None;

    return s_ShaderExtensionMap.at(ext);
}

uint32 StageToKind(EShaderStage shaderStage) {
    switch (shaderStage) {
#define GENERATE_CASE(ENUM, LOWER, UPPER, SHORT, GLSL, VULKAN)                                     \
    case EShaderStage::ENUM: return shaderc_glsl_##GLSL##_shader;
        FOREACH_SHADER_STAGE(GENERATE_CASE)
#undef GENERATE_CASE
        default: return 0;
    }
}

EShaderDataFormat GetDataFormatFromSpv(const SpvReflectTypeDescription& type) {
    if (type.type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT) {
        uint32_t component_count = type.traits.numeric.vector.component_count;
        if (type.type_flags & SPV_REFLECT_TYPE_FLAG_MATRIX) {
            uint32_t columns = type.traits.numeric.matrix.column_count;
            if (columns == 3) return EShaderDataFormat::Mat3;
            if (columns == 4) return EShaderDataFormat::Mat4;
        } else {
            if (component_count == 1) return EShaderDataFormat::Float;
            if (component_count == 2) return EShaderDataFormat::Float2;
            if (component_count == 3) return EShaderDataFormat::Float3;
            if (component_count == 4) return EShaderDataFormat::Float4;
        }
    }
    // ... 处理 Int / UInt ...
    return EShaderDataFormat::None;
}

EShaderDataFormat GetDataFormatFromSpv(const SpvReflectFormat& format) {
    switch (format) {
        // --- 32-bit Float (最常用) ---
        case SPV_REFLECT_FORMAT_R32_SFLOAT: return EShaderDataFormat::Float;
        case SPV_REFLECT_FORMAT_R32G32_SFLOAT: return EShaderDataFormat::Float2;
        case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT: return EShaderDataFormat::Float3;
        case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT: return EShaderDataFormat::Float4;

        // --- 32-bit Signed Int ---
        case SPV_REFLECT_FORMAT_R32_SINT: return EShaderDataFormat::Int;
        case SPV_REFLECT_FORMAT_R32G32_SINT: return EShaderDataFormat::Int2;
        case SPV_REFLECT_FORMAT_R32G32B32_SINT: return EShaderDataFormat::Int3;
        case SPV_REFLECT_FORMAT_R32G32B32A32_SINT: return EShaderDataFormat::Int4;

        // --- 32-bit Unsigned Int ---
        case SPV_REFLECT_FORMAT_R32_UINT: return EShaderDataFormat::UInt;
        case SPV_REFLECT_FORMAT_R32G32_UINT: return EShaderDataFormat::UInt2;
        case SPV_REFLECT_FORMAT_R32G32B32_UINT: return EShaderDataFormat::UInt3;
        case SPV_REFLECT_FORMAT_R32G32B32A32_UINT: return EShaderDataFormat::UInt4;

        // --- 16-bit Float (Half Float) ---
        // 如果引擎支持 Half 类型，可以增加对应枚举，否则暂存为 Float
        case SPV_REFLECT_FORMAT_R16_SFLOAT: return EShaderDataFormat::Float;
        case SPV_REFLECT_FORMAT_R16G16B16A16_SFLOAT: return EShaderDataFormat::Float4;

        // --- 64-bit Float (Double) ---
        // 通常实时渲染不常用，映射到 None 或根据需要处理
        case SPV_REFLECT_FORMAT_R64G64B64A64_SFLOAT: return EShaderDataFormat::None;

        case SPV_REFLECT_FORMAT_UNDEFINED:
        default: return EShaderDataFormat::None;
    }
}

EUniformType GetUniformTypeFromSpvDescType(const SpvReflectDescriptorType& spvType) {
    switch (spvType) {
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER: return EUniformType::Sampler;

        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE: return EUniformType::Image;

        case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            return EUniformType::CombinedImageSampler;

        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE: return EUniformType::StorageImage;

        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return EUniformType::UniformBuffer;

        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return EUniformType::StorageBuffer;

        case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: return EUniformType::InputAttachment;

        default:
            // CZ_LOG(LogShader, Warning, "Unknown SPIR-V descriptor type: %d", spvType);
            return EUniformType::None;
    }
}

// std::string StageToVulkanCacheFileExtension(const EShaderStage shaderStage) {
//     switch (shaderStage) {
// #define GENERATE_CASE(ENUM, LOWER, UPPER, SHORT, GLSL) \
//     case EShaderStage::ENUM: \
//         return ".cache_vulkan." + std::string(#SHORT);
//         FOREACH_SHADER_STAGE(GENERATE_CASE)
// #undef GENERATE_CASE
//     default:
//         return "";
//     }
// }

// std::filesystem::path GetCachePathByNameAndStage(const std::string_view name,
//                                                  EShaderStage stage) {
//     if (name.find_first_of("/\\") != std::string_view::npos) {
//         throw std::invalid_argument("Shader name contains path separators");
//     }

//     std::filesystem::path cacheDir =
//         ChozoUtils::File::GetShaderCacheDirectory();

//     std::filesystem::path fullPath =
//         (cacheDir / name)
//             .concat(ChozoUtils::Shader::StageToVulkanCacheFileExtension(stage))
//             .lexically_normal();

//     std::wstring pathStr = fullPath.wstring();
//     std::replace(pathStr.begin(), pathStr.end(), L'\\', L'/');

//     return std::filesystem::path(pathStr);
// }
} // namespace ChozoUtils::Shader