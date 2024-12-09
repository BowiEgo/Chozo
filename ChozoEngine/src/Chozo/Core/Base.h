#pragma once

#include <filesystem>
#include <memory>
#include <functional>
#include "Ref.h"

// #ifdef ENGINE_PLATFORM_WIN32
//     #ifdef CHOZO_BUILD_DLL
//         #define CHOZO_API __declspec(dllexport)
//     #else
//         #define CHOZO_API __declspec(dllimport)
//     #endif
// #else
//     #define CHOZO_API
// #endif

////////////////////////////////////////////////////////////////////////////
//============================ Debug =====================================//
////////////////////////////////////////////////////////////////////////////
#ifdef CZ_COMPILER_MSVC
	#define CZ_FORCE_INLINE __forceinline
#elif defined(ENGINE_PLATFORM_MACOS)
	#define CZ_FORCE_INLINE __attribute__((always_inline)) inline
#else
	#define CZ_FORCE_INLINE inline
#endif

#ifdef CZ_DEBUG
	#define CZ_ENABLE_ASSERTS
#endif

// #if defined(_MSC_VER)
//     #define DEBUG_BREAK __debugbreak()
// #elif defined(__APPLE__) || defined(__MACH__)
//     #include <signal.h>
//     #define DEBUG_BREAK __builtin_trap()
// #elif defined(__linux__)
//     #include <signal.h>
//     #define DEBUG_BREAK raise(SIGTRAP)
// #else
//     #error "Platform not supported"
// #endif

#ifdef ENGINE_PLATFORM_WIN32
	#define CZ_DEBUG_BREAK __debugbreak()
#elif defined(ENGINE_PLATFORM_MACOS)
	#define CZ_DEBUG_BREAK __builtin_debugtrap()
#elif defined(ENGINE_PLATFORM_LINUX)
    #define DEBUG_BREAK raise(SIGTRAP)
#else
	#define CZ_DEBUG_BREAK
#endif


#ifdef CZ_ENABLE_ASSERTS
    // #ifdef ENGINE_PLATFORM_MACOS
	// 	#define CZ_CORE_ASSERT_MESSAGE_INTERNAL(...)  ::Chozo::Log::PrintAssertMessage(::Chozo::Log::Type::Core, "Assertion Failed", ##__VA_ARGS__)
	// 	#define CZ_ASSERT_MESSAGE_INTERNAL(...)  ::Chozo::Log::PrintAssertMessage(::Chozo::Log::Type::Client, "Assertion Failed", ##__VA_ARGS__)
	// #else
	// 	#define CZ_CORE_ASSERT_MESSAGE_INTERNAL(...)  ::Chozo::Log::PrintAssertMessage(::Chozo::Log::Type::Core, "Assertion Failed" __VA_OPT__(,) __VA_ARGS__)
	// 	#define CZ_ASSERT_MESSAGE_INTERNAL(...)  ::Chozo::Log::PrintAssertMessage(::Chozo::Log::Type::Client, "Assertion Failed" __VA_OPT__(,) __VA_ARGS__)
	// #endif

    #define CZ_ASSERT(condition, ...) { if(!(condition)) { CZ_ERROR("Assertion Failed: {0}", __VA_ARGS__); CZ_DEBUG_BREAK; } }
    #define CZ_CORE_ASSERT(condition, ...) { if(!(condition)) { CZ_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); CZ_DEBUG_BREAK; } }
    // #define CZ_CORE_ASSERT(condition, ...) { if(!(condition)) { CZ_CORE_ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); CZ_DEBUG_BREAK; } }
	// #define CZ_ASSERT(condition, ...) { if(!(condition)) { CZ_ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); CZ_DEBUG_BREAK; } }
#else
    #define CZ_ASSERT(condition, ...)
    #define CZ_CORE_ASSERT(condition, ...)
#endif

#define BIT(x) (1 << x)

#define CZ_BIND_EVENT_FN(fn) [this](auto && PH1) { return fn(std::forward<decltype(PH1)>(PH1)); }

namespace Chozo {
	namespace fs = std::filesystem;
	
	using byte = uint8_t;

    template<typename T, typename ... Args>
	using Callback = std::function<T(Args&& ...args)>;

    template<typename T>
    using Scope = std::unique_ptr<T>;

    template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

}