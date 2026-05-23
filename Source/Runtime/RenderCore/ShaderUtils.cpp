#include "ShaderUtils.hpp"

#include <Core/Utilities/StringUtils.hpp>
#include <Runtime/RHI/RHITypes.hpp>

#include <string>
#include <unordered_map>

namespace CZ::ShaderUtils {

const std::unordered_map<std::string, ShaderStage> s_ShaderExtensionMap = {
#define GENERATE_MAP(ENUM, LOWER, UPPER, SHORT, GLSL, VULKAN, VULKAN_UPPER)                        \
    { "." #SHORT, ShaderStage::ENUM },
    FOREACH_SHADER_STAGE(GENERATE_MAP)
#undef GENERATE_MAP
        { ".pixel", ShaderStage::Fragment }
};

ShaderStage StringToStage(std::string_view shaderStage) {
#define GENERATE_IF(ENUM, LOWER, UPPER, SHORT, GLSL, VULKAN, VULKAN_UPPER)                         \
    if (shaderStage == #ENUM) return ShaderStage::ENUM;
    FOREACH_SHADER_STAGE(GENERATE_IF)
#undef GENERATE_IF
    return ShaderStage::None;
}

const std::string StageToString(ShaderStage shaderStage, bool bUpper) {
    switch (shaderStage) {
#define GENERATE_CASE(ENUM, LOWER, UPPER, SHORT, GLSL, VULKAN, VULKAN_UPPER)                       \
    case ShaderStage::ENUM: return bUpper ? #UPPER : #ENUM;
        FOREACH_SHADER_STAGE(GENERATE_CASE)
#undef GENERATE_CASE
        default: return "Unknown";
    }
}

ShaderStage GetStageFromExtension(const std::string& extension) {
    const std::string ext = StringUtils::ToLowerCopy(extension);
    if (s_ShaderExtensionMap.find(ext) == s_ShaderExtensionMap.end()) return ShaderStage::None;

    return s_ShaderExtensionMap.at(ext);
}

ShaderStage GetShaderStageFromSlangStage(const SlangStage slangStage) {
    switch (slangStage) {
        case SLANG_STAGE_VERTEX: return ShaderStage::Vertex;
        case SLANG_STAGE_HULL: return ShaderStage::Hull;
        case SLANG_STAGE_DOMAIN: return ShaderStage::Domain;
        case SLANG_STAGE_GEOMETRY: return ShaderStage::Geometry;
        case SLANG_STAGE_FRAGMENT: return ShaderStage::Fragment;
        case SLANG_STAGE_COMPUTE: return ShaderStage::Compute;
        default: return ShaderStage::None;
    }
}

// uint32 StageToKind(ShaderStage shaderStage) {
//     switch (shaderStage) {
// #define GENERATE_CASE(ENUM, LOWER, UPPER, SHORT, GLSL, VULKAN, VULKAN_UPPER) \
//     case ShaderStage::ENUM: return shaderc_glsl_##GLSL##_shader;
//         FOREACH_SHADER_STAGE(GENERATE_CASE)
// #undef GENERATE_CASE
//         default: return 0;
//     }
// }

// ShaderDataType GetDataFormatFromSpv(const SpvReflectTypeDescription& type) {
//     if (type.type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT) {
//         uint32_t component_count = type.traits.numeric.vector.component_count;
//         if (type.type_flags & SPV_REFLECT_TYPE_FLAG_MATRIX) {
//             uint32_t columns = type.traits.numeric.matrix.column_count;
//             if (columns == 3) return ShaderDataType::Mat3;
//             if (columns == 4) return ShaderDataType::Mat4;
//         } else {
//             if (component_count == 1) return ShaderDataType::Float;
//             if (component_count == 2) return ShaderDataType::Float2;
//             if (component_count == 3) return ShaderDataType::Float3;
//             if (component_count == 4) return ShaderDataType::Float4;
//         }
//     }
//     // ... 处理 Int / UInt ...
//     return ShaderDataType::None;
// }

// ShaderDataType GetDataFormatFromSpv(const SpvReflectFormat& format) {
//     switch (format) {
//         // --- 32-bit Float (最常用) ---
//         case SPV_REFLECT_FORMAT_R32_SFLOAT: return ShaderDataType::Float;
//         case SPV_REFLECT_FORMAT_R32G32_SFLOAT: return ShaderDataType::Float2;
//         case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT: return ShaderDataType::Float3;
//         case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT: return ShaderDataType::Float4;

//         // --- 32-bit Signed Int ---
//         case SPV_REFLECT_FORMAT_R32_SINT: return ShaderDataType::Int;
//         case SPV_REFLECT_FORMAT_R32G32_SINT: return ShaderDataType::Int2;
//         case SPV_REFLECT_FORMAT_R32G32B32_SINT: return ShaderDataType::Int3;
//         case SPV_REFLECT_FORMAT_R32G32B32A32_SINT: return ShaderDataType::Int4;

//         // --- 32-bit Unsigned Int ---
//         case SPV_REFLECT_FORMAT_R32_UINT: return ShaderDataType::UInt;
//         case SPV_REFLECT_FORMAT_R32G32_UINT: return ShaderDataType::UInt2;
//         case SPV_REFLECT_FORMAT_R32G32B32_UINT: return ShaderDataType::UInt3;
//         case SPV_REFLECT_FORMAT_R32G32B32A32_UINT: return ShaderDataType::UInt4;

//         // --- 16-bit Float (Half Float) ---
//         // 如果引擎支持 Half 类型，可以增加对应枚举，否则暂存为 Float
//         case SPV_REFLECT_FORMAT_R16_SFLOAT: return ShaderDataType::Float;
//         case SPV_REFLECT_FORMAT_R16G16B16A16_SFLOAT: return ShaderDataType::Float4;

//         // --- 64-bit Float (Double) ---
//         // 通常实时渲染不常用，映射到 None 或根据需要处理
//         case SPV_REFLECT_FORMAT_R64G64B64A64_SFLOAT: return ShaderDataType::None;

//         case SPV_REFLECT_FORMAT_UNDEFINED:
//         default: return ShaderDataType::None;
//     }
// }

// UniformType GetUniformTypeFromSpvDescType(const SpvReflectDescriptorType& spvType) {
//     switch (spvType) {
//         case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER: return UniformType::Sampler;

//         case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE: return UniformType::Image;

//         case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
//             return UniformType::CombinedImageSampler;

//         case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE: return UniformType::StorageImage;

//         case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
//         case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return
//         UniformType::UniformBuffer;

//         case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
//         case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return
//         UniformType::StorageBuffer;

//         case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: return UniformType::InputAttachment;

//         default:
//             // CZ_LOG(LogShader, Warning, "Unknown SPIR-V descriptor type: %d", spvType);
//             return UniformType::None;
//     }
// }

} // namespace CZ::ShaderUtils
