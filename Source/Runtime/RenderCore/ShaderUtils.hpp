#pragma once

#include <Runtime/RHI/RHITypes.hpp>

#include "slang.h"

namespace CZ::ShaderUtils {

ShaderStage StringToStage(std::string_view shaderStage);

const std::string StageToString(ShaderStage shaderStage, bool bUpper = false);

ShaderStage GetStageFromExtension(const std::string& extension);

ShaderStage GetShaderStageFromSlangStage(const SlangStage slangStage);

// uint32 StageToKind(ShaderStage shaderStage);

// ShaderDataType GetDataFormatFromSpv(const SpvReflectTypeDescription& type);
// ShaderDataType GetDataFormatFromSpv(const SpvReflectFormat& format);

// UniformType GetUniformTypeFromSpvDescType(const SpvReflectDescriptorType& spvType);

// std::string StageToVulkanCacheFileExtension(const ShaderStage shaderStage);

// std::filesystem::path GetCachePathByNameAndStage(const std::string_view name,
//                                                  ShaderStage stage);

} // namespace CZ::ShaderUtils
