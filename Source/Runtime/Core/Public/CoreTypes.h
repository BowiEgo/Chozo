#pragma once

#include <functional>

////////////////////////////////////////////////////////////////////////////
//=========================== Global Macros ==============================//
////////////////////////////////////////////////////////////////////////////

// Use 1ULL to ensure we support up to 64-bit flags safely
#define BIT(x) (1ULL << (x))

// Support functions with any number of arguments
#define CZ_BIND_EVENT_FN(fn)                                                   \
    [this](auto &&...args) { return fn(std::forward<decltype(args)>(args)...); }

////////////////////////////////////////////////////////////////////////////
//============================ Global Scope ==============================//
////////////////////////////////////////////////////////////////////////////

template <typename T, typename... Args>
using TCallback = std::function<T(Args &&...args)>;

////////////////////////////////////////////////////////////////////////////
//========================== Basic Type Aliases ==========================//
////////////////////////////////////////////////////////////////////////////

// Stick to explicit sizes for global basic types to avoid ambiguity
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;

using f32 = float;
using f64 = double;

using FByte = uint8;
using FUint = uint32;
