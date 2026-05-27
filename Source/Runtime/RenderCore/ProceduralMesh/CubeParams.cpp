#include <Runtime/RenderCore/ProceduralMesh/CubeParams.hpp>

namespace CZ {

inline const ParamControllerConfig kSizeConfig{ .Type  = ParamControllerType::Drag,
                                                .Min   = 0.0f,
                                                .Speed = 0.1 };
inline const ParamControllerConfig kSegmentsConfig{ .Type  = ParamControllerType::Drag,
                                                    .Min   = 0.0f,
                                                    .Speed = 1 };

#define PARAMS_LIST                                                                                \
    PARAM(float, Width, "Width", kSizeConfig)                                                      \
    PARAM(float, Height, "Height", kSizeConfig)                                                    \
    PARAM(float, Depth, "Depth", kSizeConfig)                                                      \
    PARAM(uint32_t, WidthSegments, "Width Segments", kSegmentsConfig)                              \
    PARAM(uint32_t, HeightSegments, "Height Segments", kSegmentsConfig)                            \
    PARAM(uint32_t, DepthSegments, "Depth Segments", kSegmentsConfig)

size_t CubeParams::GetHash() const {
    size_t h = 0;
#define PARAM(type, member, ...) HashCombine(h, std::hash<type>{}(member));
    PARAMS_LIST
#undef PARAM
    return h;
}

std::any CubeParams::GetParamValue(const std::string& name) const {
#define PARAM(type, member, ...)                                                                   \
    if (name == #member) return member;
    PARAMS_LIST
#undef PARAM
    return {};
}

// ===== IMaterialParams Implementation =====
bool CubeParams::Equals_Internal(const Params& other) const {
    const auto* otherMat = dynamic_cast<const CubeParams*>(&other);
    if (!otherMat) return false;

#define PARAM(type, member, ...)                                                                   \
    if (member != otherMat->member) return false;
    PARAMS_LIST
#undef PARAM

    return true;
}

void CubeParams::Accept_Internal(ParamsVisitor& visitor) {
#define PARAM(type, member, display, config, ...) visitor.Visit(member, display, config);
    PARAMS_LIST
#undef PARAM
}

void CubeParams::Accept_Internal(ConstParamsVisitor& visitor) const {
#define PARAM(type, member, display, config, ...) visitor.Visit(member, display, config);
    PARAMS_LIST
#undef PARAM
}

const std::vector<std::string>& CubeParams::GetAllParamNames_Internal() {
    static const std::vector<std::string> names = [] {
        std::vector<std::string> result;
#define PARAM(type, member, display, ...) result.push_back(#member);
        PARAMS_LIST
#undef PARAM
        return result;
    }();
    return names;
}

} // namespace CZ
