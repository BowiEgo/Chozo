#include "CubeParams.h"

inline const FParamControllerConfig kSizeConfig{ .Type  = EParamControllerType::Drag,
                                                 .Min   = 0.0f,
                                                 .Speed = 0.1 };
inline const FParamControllerConfig kSegmentsConfig{ .Type  = EParamControllerType::Drag,
                                                     .Min   = 0.0f,
                                                     .Speed = 1 };

#define PARAMS_LIST                                                                                \
    PARAM(float, Width, "Width", kSizeConfig)                                                      \
    PARAM(float, Height, "Height", kSizeConfig)                                                    \
    PARAM(float, Depth, "Depth", kSizeConfig)                                                      \
    PARAM(uint32_t, WidthSegments, "Width Segments", kSegmentsConfig)                              \
    PARAM(uint32_t, HeightSegments, "Height Segments", kSegmentsConfig)                            \
    PARAM(uint32_t, DepthSegments, "Depth Segments", kSegmentsConfig)

size_t FCubeParams::GetHash() const {
    size_t h = 0;
#define PARAM(type, member, ...) HashCombine(h, std::hash<type>{}(member));
    PARAMS_LIST
#undef PARAM
    return h;
}

std::any FCubeParams::GetParamValue(const std::string& name) const {
#define PARAM(type, member, ...)                                                                   \
    if (name == #member) return member;
    PARAMS_LIST
#undef PARAM
    return {};
}

// ===== IMaterialParams Implementation =====
bool FCubeParams::Equals_Internal(const IParams& other) const {
    const auto* otherMat = dynamic_cast<const FCubeParams*>(&other);
    if (!otherMat) return false;

#define PARAM(type, member, ...)                                                                   \
    if (member != otherMat->member) return false;
    PARAMS_LIST
#undef PARAM

    return true;
}

void FCubeParams::Accept_Internal(IParamsVisitor& visitor) {
#define PARAM(type, member, display, config, ...) visitor.Visit(member, display, config);
    PARAMS_LIST
#undef PARAM
}

void FCubeParams::Accept_Internal(IConstParamsVisitor& visitor) const {
#define PARAM(type, member, display, config, ...) visitor.Visit(member, display, config);
    PARAMS_LIST
#undef PARAM
}

const std::vector<std::string>& FCubeParams::GetAllParamNames_Internal() {
    static const std::vector<std::string> names = [] {
        std::vector<std::string> result;
#define PARAM(type, member, display, ...) result.push_back(#member);
        PARAMS_LIST
#undef PARAM
        return result;
    }();
    return names;
}