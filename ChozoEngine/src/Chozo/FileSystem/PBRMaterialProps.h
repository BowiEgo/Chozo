#pragma once

#include "czpch.h"
#include "Chozo/Utilities/StringUtils.h"

namespace Chozo {

    #define FOREACH_PBRMATERIALTEXTURE_TYPE(TYPE) \
            TYPE(BaseColor) \
            TYPE(Metallic) \
            TYPE(Roughness) \
            TYPE(MetallicRoughness) \
            TYPE(Normal) \
            TYPE(Occlusion) \
            TYPE(Emissive) \

    enum class PBRMaterialTextureType : uint16_t {
        #define GENERATE_ENUM(ENUM) ENUM,
        FOREACH_PBRMATERIALTEXTURE_TYPE(GENERATE_ENUM)
        #undef GENERATE_ENUM
        None
    };

    inline PBRMaterialTextureType StringToPBRMaterialTextureType(std::string_view propType) {
        #define GENERATE_IF(ENUM) if (propType == #ENUM) return PBRMaterialTextureType::ENUM;
        FOREACH_PBRMATERIALTEXTURE_TYPE(GENERATE_IF)
        #undef GENERATE_IF
        return PBRMaterialTextureType::None;
    }

    inline const char* PBRMaterialTextureTypeToString(PBRMaterialTextureType propType) {
        switch (propType) {
            #define GENERATE_CASE(ENUM) case PBRMaterialTextureType::ENUM: return #ENUM;
            FOREACH_PBRMATERIALTEXTURE_TYPE(GENERATE_CASE)
            #undef GENERATE_CASE
            default: return "Unknown";
        }
    }
}