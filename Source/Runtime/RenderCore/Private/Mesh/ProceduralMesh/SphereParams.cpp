#include "SphereParams.h"

inline const FParamControllerConfig kRadiusConfig{ .Type = EParamControllerType::Drag,
                                                   .Min  = 0.0f };
inline const FParamControllerConfig kSegmentsConfig{ .Type  = EParamControllerType::Drag,
                                                     .Min   = 0.0f,
                                                     .Speed = 1 };
inline const FParamControllerConfig kPiConfig{ .Type = EParamControllerType::Slider,
                                               .Min  = 0.0f,
                                               .Max  = ChozoUtils::Math::PI * 2 };

#define PARAMS_LIST                                                                                \
    PARAM(float, Radius, "Radius", kRadiusConfig)                                                  \
    PARAM(uint32_t, WidthSegments, "Width Segments", kSegmentsConfig)                              \
    PARAM(uint32_t, HeightSegments, "Height Segments", kSegmentsConfig)                            \
    PARAM(float, PhiStart, "Phi Start", kPiConfig)                                                 \
    PARAM(float, PhiLength, "Phi Length", kPiConfig)                                               \
    PARAM(float, ThetaStart, "Theta Start", kPiConfig)                                             \
    PARAM(float, ThetaLength, "Theta Length", kPiConfig)

size_t FSphereParams::GetHash() const {
    size_t h = 0;
#define PARAM(type, member, ...) HashCombine(h, std::hash<type>{}(member));
    PARAMS_LIST
#undef PARAM
    return h;
}

std::any FSphereParams::GetParamValue(const std::string& name) const {
#define PARAM(type, member, ...)                                                                   \
    if (name == #member) return member;
    PARAMS_LIST
#undef PARAM
    return {};
}

// ===== IMaterialParams Implementation =====
bool FSphereParams::Equals_Internal(const IParams& other) const {
    const auto* otherMat = dynamic_cast<const FSphereParams*>(&other);
    if (!otherMat) return false;

#define PARAM(type, member, ...)                                                                   \
    if (member != otherMat->member) return false;
    PARAMS_LIST
#undef PARAM

    return true;
}

void FSphereParams::Accept_Internal(IParamsVisitor& visitor) {
#define PARAM(type, member, display, config, ...) visitor.Visit(member, display, config);
    PARAMS_LIST
#undef PARAM
}

void FSphereParams::Accept_Internal(IConstParamsVisitor& visitor) const {
#define PARAM(type, member, display, config, ...) visitor.Visit(member, display, config);
    PARAMS_LIST
#undef PARAM
}

const std::vector<std::string>& FSphereParams::GetAllParamNames_Internal() {
    static const std::vector<std::string> names = [] {
        std::vector<std::string> result;
#define PARAM(type, member, display, ...) result.push_back(#member);
        PARAMS_LIST
#undef PARAM
        return result;
    }();
    return names;
}