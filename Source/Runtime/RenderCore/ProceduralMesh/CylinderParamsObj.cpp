#include <any>
#include <functional>
#include <string>
#include <vector>

#include <cstddef>
#include <cstdint>

#include "Runtime/RenderCore/MeshParams.hpp"
#include <Runtime/RenderCore/ProceduralMesh/CylinderParamsObj.hpp>

namespace CZ {

inline const ParamControllerConfig kSizeConfig{ .Type  = ParamControllerType::Drag,
                                                .Min   = 0.0f,
                                                .Speed = 0.1 };
inline const ParamControllerConfig kSegmentsConfig{ .Type  = ParamControllerType::Drag,
                                                    .Min   = 0.0f,
                                                    .Speed = 1 };

#define PARAMS_LIST                                                                                \
    PARAM(float, Radius, "Radius", kSizeConfig)                                                    \
    PARAM(float, Height, "Height", kSizeConfig)                                                    \
    PARAM(uint32_t, RadialSegments, "Radial Segments", kSegmentsConfig)                            \
    PARAM(uint32_t, HeightSegments, "Height Segments", kSegmentsConfig)

size_t CylinderParamsObj::GetHash() const {
    size_t h = 0;
#define PARAM(type, member, ...) HashCombine(h, std::hash<type>{}(member));
    PARAMS_LIST
#undef PARAM
    return h;
}

std::any CylinderParamsObj::GetParamValue(const std::string& name) const {
#define PARAM(type, member, ...)                                                                   \
    if (name == #member) return member;
    PARAMS_LIST
#undef PARAM
    return {};
}

// ===== IMaterialParams Implementation =====
bool CylinderParamsObj::Equals_Internal(MeshParamsObj& other) {
    auto* otherMat = dynamic_cast<CylinderParamsObj*>(&other);
    if (!otherMat) return false;

#define PARAM(type, member, ...)                                                                   \
    if (member != otherMat->member) return false;
    PARAMS_LIST
#undef PARAM

    return true;
}

bool CylinderParamsObj::Equals_Internal(const MeshParamsObj& other) const {
    const auto* otherMat = dynamic_cast<const CylinderParamsObj*>(&other);
    if (!otherMat) return false;

#define PARAM(type, member, ...)                                                                   \
    if (member != otherMat->member) return false;
    PARAMS_LIST
#undef PARAM

    return true;
}

void CylinderParamsObj::Accept_Internal(ParamsVisitor& visitor) {
#define PARAM(type, member, display, config, ...) visitor.Visit(member, display, config);
    PARAMS_LIST
#undef PARAM
}

void CylinderParamsObj::Accept_Internal(ConstParamsVisitor& visitor) const {
#define PARAM(type, member, display, config, ...) visitor.Visit(member, display, config);
    PARAMS_LIST
#undef PARAM
}

const std::vector<std::string>& CylinderParamsObj::GetAllParamNames_Internal() {
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
