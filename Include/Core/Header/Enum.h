#pragma once

#include <type_traits>

////////////////////////////////////////////////////////////////////////////
//================================ Enum ==================================//
////////////////////////////////////////////////////////////////////////////
/**
 * ENUM_CLASS_FLAGS - Macro to enable bitwise operations for enum classes.
 * Includes: |, &, ^, ~, |=, &=, ^=
 */
#define ENUM_CLASS_FLAGS(Enum)                                                                     \
    inline Enum operator|(Enum a, Enum b) {                                                        \
        return static_cast<Enum>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));             \
    }                                                                                              \
    inline Enum operator&(Enum a, Enum b) {                                                        \
        return static_cast<Enum>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));             \
    }                                                                                              \
    inline Enum operator^(Enum a, Enum b) {                                                        \
        return static_cast<Enum>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));             \
    }                                                                                              \
    inline Enum operator~(Enum a) { return static_cast<Enum>(~static_cast<uint32_t>(a)); }         \
    inline Enum& operator|=(Enum& a, Enum b) {                                                     \
        return reinterpret_cast<Enum&>(reinterpret_cast<uint32_t&>(a) |=                           \
                                       static_cast<uint32_t>(b));                                  \
    }                                                                                              \
    inline Enum& operator&=(Enum& a, Enum b) {                                                     \
        return reinterpret_cast<Enum&>(reinterpret_cast<uint32_t&>(a) &=                           \
                                       static_cast<uint32_t>(b));                                  \
    }                                                                                              \
    inline Enum& operator^=(Enum& a, Enum b) {                                                     \
        return reinterpret_cast<Enum&>(reinterpret_cast<uint32_t&>(a) ^=                           \
                                       static_cast<uint32_t>(b));                                  \
    }

/**
 * Checks if any of the specified flags are set in the given bitmask.
 * Works with any enum class that has an underlying uint32_t type.
 */
template <typename Enum> [[nodiscard]] inline bool HasFlag(Enum flags, Enum contains) {
    // static_cast to the underlying type (usually uint32_t) for bitwise comparison
    using UnderlyingType = typename std::underlying_type<Enum>::type;
    return (static_cast<UnderlyingType>(flags) & static_cast<UnderlyingType>(contains)) != 0;
}

/**
 * Checks if all of the specified flags are set in the given bitmask.
 */
template <typename Enum> [[nodiscard]] inline bool HasAllFlags(Enum flags, Enum contains) {
    using UnderlyingType = typename std::underlying_type<Enum>::type;
    return (static_cast<UnderlyingType>(flags) & static_cast<UnderlyingType>(contains)) ==
           static_cast<UnderlyingType>(contains);
}
