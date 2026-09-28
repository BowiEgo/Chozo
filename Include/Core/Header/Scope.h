#pragma once
#include <cstdint>

#include <functional>

////////////////////////////////////////////////////////////////////////////
//============================ Global Scope ==============================//
////////////////////////////////////////////////////////////////////////////
template <typename T, typename... Args> using TCallback = std::function<T(Args&&... args)>;
using CallbackHandle                                    = uint32_t;