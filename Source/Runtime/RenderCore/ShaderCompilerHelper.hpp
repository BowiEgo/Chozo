#pragma once

#include <Core/Log/LogMacros.hpp>
#include <Runtime/RHI/RHITypes.hpp>

#include "slang.h"

namespace CZ {

#define RETURN_ON_FAIL(result)                                                                     \
    do {                                                                                           \
        SlangResult _res = (result);                                                               \
        if (SLANG_FAILED(_res)) {                                                                  \
            CZ_CORE_LOG(Error, "Slang error: {}", _res);                                           \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

#define WITH_ARRAY() for (int _i = (beginArray(), 1); _i; _i = (endArray(), 0))

inline void DiagnoseIfNeeded(slang::IBlob* diagnosticBlob) {
    if (diagnosticBlob != nullptr) {
        CZ_CORE_LOG(Error, "Slang loadModule error: {}",
                    (const char*)diagnosticBlob->getBufferPointer());
    }
}

inline UniformType GetUniformTypeFromSlangCategory(slang::TypeLayoutReflection* typeLayout) {
    using namespace slang;

    ParameterCategory category = typeLayout->getParameterCategory();

    if (category == ParameterCategory::DescriptorTableSlot) {
        slang::TypeLayoutReflection* contentLayout = typeLayout->getElementTypeLayout();
        if (contentLayout) {
            category   = contentLayout->getParameterCategory();
            typeLayout = contentLayout;
        }
    }

    switch (category) {
        case ParameterCategory::Uniform:
        case ParameterCategory::ConstantBuffer: return UniformType::UniformBuffer;

        case ParameterCategory::SamplerState: return UniformType::Sampler;

        case ParameterCategory::PushConstantBuffer:
        case ParameterCategory::SpecializationConstant: return UniformType::PushConstant;

        case ParameterCategory::ShaderResource: {
            auto* type = typeLayout->getType();
            if (type->getKind() == TypeReflection::Kind::Resource) {
                SlangResourceShape shape = type->getResourceShape();
                if (shape == SLANG_STRUCTURED_BUFFER || shape == SLANG_BYTE_ADDRESS_BUFFER) {
                    return UniformType::StorageBuffer;
                }
            }
            return UniformType::Image;
        }

        case ParameterCategory::UnorderedAccess: {
            auto* type = typeLayout->getType();
            if (type->getKind() == TypeReflection::Kind::Resource) {
                SlangResourceShape shape = type->getResourceShape();
                if (shape == SLANG_STRUCTURED_BUFFER || shape == SLANG_BYTE_ADDRESS_BUFFER) {
                    return UniformType::StorageBuffer;
                }
            }
            return UniformType::StorageImage;
        }

        default: break;
    }

    // special case for input attachments (Vulkan-specific)
    // if shader variable has [[vk::input_attachment_index]] modifier, we treat it as
    // InputAttachment if (varLayout->findModifier(slang::Modifier::InputAttachmentIndex))
    // return UniformType::InputAttachment;

    return UniformType::None;
}

inline ShaderDataType SlangTypeToShaderDataType(slang::TypeReflection* type) {
    if (!type) return ShaderDataType::None;

    using namespace slang;
    switch (type->getKind()) {
        case TypeReflection::Kind::Scalar: {
            switch (type->getScalarType()) {
                case TypeReflection::ScalarType::Float32: return ShaderDataType::Float;
                case TypeReflection::ScalarType::Int32: return ShaderDataType::Int;
                case TypeReflection::ScalarType::UInt32: return ShaderDataType::UInt;
                case TypeReflection::ScalarType::Bool: return ShaderDataType::Bool;
                default: break;
            }
            break;
        }
        case TypeReflection::Kind::Vector: {
            uint32_t count = type->getElementCount();
            switch (type->getScalarType()) {
                case TypeReflection::ScalarType::Float32:
                    if (count == 2) return ShaderDataType::Float2;
                    if (count == 3) return ShaderDataType::Float3;
                    if (count == 4) return ShaderDataType::Float4;
                    break;
                case TypeReflection::ScalarType::Int32:
                    if (count == 2) return ShaderDataType::Int2;
                    if (count == 3) return ShaderDataType::Int3;
                    if (count == 4) return ShaderDataType::Int4;
                    break;
                case TypeReflection::ScalarType::UInt32:
                    if (count == 2) return ShaderDataType::UInt2;
                    if (count == 3) return ShaderDataType::UInt3;
                    if (count == 4) return ShaderDataType::UInt4;
                    break;
                default: break;
            }
            break;
        }
        case TypeReflection::Kind::Matrix: {
            if (type->getScalarType() == TypeReflection::ScalarType::Float32) {
                uint32_t rows = type->getRowCount();
                uint32_t cols = type->getColumnCount();
                if (rows == 4 && cols == 4) return ShaderDataType::Mat4;
                if (rows == 3 && cols == 3) return ShaderDataType::Mat3;
            }
            break;
        }
        default: break;
    }
    return ShaderDataType::None;
}
} // namespace CZ