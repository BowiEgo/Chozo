#include "PBRMaterialParams.h"

inline const FParamControllerConfig kBaseColorConfig{ .Type = EParamControllerType::ColorPicker };
inline const FParamControllerConfig kMetallicConfig{ .Type = EParamControllerType::Slider,
                                                     0.0f,
                                                     1.0f };
inline const FParamControllerConfig kRoughnessConfig{ .Type = EParamControllerType::Slider,
                                                      0.0f,
                                                      1.0f };
inline const FParamControllerConfig kNormalStrengthConfig{ .Type = EParamControllerType::Slider,
                                                           0.0f,
                                                           2.0f };
inline const FParamControllerConfig kEmissiveStrengthConfig{ .Type = EParamControllerType::Slider,
                                                             0.0,
                                                             1.0f };
inline const FParamControllerConfig kMapConfig{ .Type         = EParamControllerType::AssetPicker,
                                                .bNotifyDirty = false };
inline const FParamControllerConfig kCheckboxConfig{ .Type = EParamControllerType::Checkbox };

#define PARAMS_LIST                                                                                \
    PARAM(FVector4, BaseColor, "Base Color", kBaseColorConfig)                                     \
    PARAM(float, Metallic, "Metallic", kMetallicConfig)                                            \
    PARAM(float, Roughness, "Roughness", kRoughnessConfig)                                         \
    PARAM(float, NormalStrength, "Normal Strength", kNormalStrengthConfig)                         \
    PARAM(float, EmissiveStrength, "Emissive Strength", kEmissiveStrengthConfig)                   \
    PARAM(FAssetHandle, AlbedoMap, "Albedo Map", kMapConfig)                                       \
    PARAM(FAssetHandle, NormalMap, "Normal Map", kMapConfig)                                       \
    PARAM(FAssetHandle, RMAOMap, "RMAO Map", kMapConfig)                                           \
    PARAM(bool, UseAlbedoMap, "Use Albedo Map", kCheckboxConfig)                                   \
    PARAM(bool, UseNormalMap, "Use Normal Map", kCheckboxConfig)                                   \
    PARAM(bool, UseRMAOMap, "Use RMAO Map", kCheckboxConfig)

size_t FPBRMaterialParams::GetHash() const {
    size_t h = 0;
#define PARAM(type, member, ...) HashCombine(h, std::hash<type>{}(member));
    PARAMS_LIST
#undef PARAM
    return h;
}

std::any FPBRMaterialParams::GetParamValue(const std::string& name) const {
#define PARAM(type, member, ...)                                                                   \
    if (name == #member) return member;
    PARAMS_LIST
#undef PARAM
    return {};
}

// ===== IMaterialParams Implementation =====
bool FPBRMaterialParams::Equals_Internal(const IParams& other) const {
    const auto* otherMat = dynamic_cast<const FPBRMaterialParams*>(&other);
    if (!otherMat) return false;

#define PARAM(type, member, ...)                                                                   \
    if (member != otherMat->member) return false;
    PARAMS_LIST
#undef PARAM

    return true;
}

void FPBRMaterialParams::Accept_Internal(IParamsVisitor& visitor) {
#define PARAM(type, member, display, config, ...) visitor.Visit(member, display, config);
    PARAMS_LIST
#undef PARAM
}

void FPBRMaterialParams::Accept_Internal(IConstParamsVisitor& visitor) const {
#define PARAM(type, member, display, config, ...) visitor.Visit(member, display, config);
    PARAMS_LIST
#undef PARAM
}

const std::vector<std::string>& FPBRMaterialParams::GetAllParamNames_Internal() {
    static const std::vector<std::string> names = [] {
        std::vector<std::string> result;
#define PARAM(type, member, display, ...) result.push_back(#member);
        PARAMS_LIST
#undef PARAM
        return result;
    }();
    return names;
}