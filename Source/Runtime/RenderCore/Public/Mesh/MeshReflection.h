#pragma once

#include <optional>
#include <type_traits>
#include <variant>

namespace MeshProp {
struct Radius {};
struct Size {};
struct Width {};
struct Height {};
struct Depth {};
struct Segments {};
struct WidthSegments {};
struct HeightSegments {};
struct DepthSegments {};
struct PhiStart {};
struct PhiLength {};
struct ThetaStart {};
struct ThetaLength {};
} // namespace MeshProp

template <typename Tag> struct MeshPropertyTraits;

// ===== Macro: Generate property traits =====
#define DEFINE_PROPERTY_TRAITS(ParamsType, PropName, PropType, TagName)                            \
    template <> struct MeshPropertyTraits<MeshProp::TagName> {                                     \
        using Type                        = PropType;                                              \
        static constexpr const char* Name = #PropName;                                             \
        static void Set(ParamsType& params, PropType value) { params.PropName = value; }           \
        static PropType Get(const ParamsType& params) { return params.PropName; }                  \
    };

// ===== Macro: Generate SetProperty function =====
#define DEFINE_SET_PROPERTY(ParamsType, PropName, PropType, TagName)                               \
    inline bool SetProperty(ParamsType& params, MeshProp::TagName, PropType value) {               \
        if (params.PropName != value) {                                                            \
            params.PropName = value;                                                               \
            return true;                                                                           \
        }                                                                                          \
        return false;                                                                              \
    }

// ===== Macro: Generate GetParamValue function =====
#define DEFINE_GET_PROPERTY(ParamsType, PropName, PropType, TagName)                               \
    inline std::optional<PropType> GetParamValue(const ParamsType& params, MeshProp::TagName) {    \
        return params.PropName;                                                                    \
    }

// ===== Macro: Generate CreateFromProperty function =====
#define DEFINE_CREATE_FROM_PROPERTY(ParamsType, PropName, PropType, TagName, ...)                  \
    inline ParamsType CreateFromProperty(MeshProp::TagName, PropType value) {                      \
        ParamsType params;                                                                         \
        params.PropName = value;                                                                   \
        __VA_ARGS__                                                                                \
        return params;                                                                             \
    }

// ===== Combined macro: Generate everything in one =====
#define DEFINE_MESH_PROPERTY(ParamsType, PropName, PropType, TagName, ...)                         \
    DEFINE_PROPERTY_TRAITS(ParamsType, PropName, PropType, TagName)                                \
    DEFINE_SET_PROPERTY(ParamsType, PropName, PropType, TagName)                                   \
    DEFINE_GET_PROPERTY(ParamsType, PropName, PropType, TagName)                                   \
    DEFINE_CREATE_FROM_PROPERTY(ParamsType, PropName, PropType, TagName, __VA_ARGS__)

// ===== Simplified version: Only generate traits and operations (most common) =====
#define SIMPLE_MESH_PROPERTY(ParamsType, PropName, PropType, TagName)                              \
    DEFINE_PROPERTY_TRAITS(ParamsType, PropName, PropType, TagName)                                \
    DEFINE_SET_PROPERTY(ParamsType, PropName, PropType, TagName)                                   \
    DEFINE_GET_PROPERTY(ParamsType, PropName, PropType, TagName)

#define DECLARE_PARAM_COPY_CTOR(ParamsType)                                                        \
    ParamsType(const ParamsType&)            = default;                                            \
    ParamsType& operator=(const ParamsType&) = default;

#define DECLARE_PARAM_COPY_CTOR_EXPLICIT(ParamsType, ...)                                          \
    ParamsType(const ParamsType& other) : __VA_ARGS__ {}                                           \
    ParamsType& operator=(const ParamsType& other) {                                               \
        __VA_ARGS__;                                                                               \
        return *this;                                                                              \
    }

#define DECLARE_PARAM_COPY_CTOR_SIMPLE(ParamsType, MemberList)                                     \
    ParamsType(const ParamsType& other) : MemberList {}                                            \
    ParamsType& operator=(const ParamsType& other) {                                               \
        MemberList;                                                                                \
        return *this;                                                                              \
    }