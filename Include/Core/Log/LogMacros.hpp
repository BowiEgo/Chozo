#pragma once

#include <Core/Log/Logger.hpp>

#include <fmt/core.h>

namespace CZ {

/**
 * Base class for all log categories
 */
struct LogCategoryBase {
    LogCategoryBase(const char* InName, LogVerbosity InDefaultVerbosity)
        : Name(InName), DefaultVerbosity(InDefaultVerbosity) {}

    const char* Name;
    LogVerbosity DefaultVerbosity;
};

} // namespace CZ

#ifdef CZ_DIST
    #define CZ_LOG_LEVEL LogVerbosity::Info
#else
    #define CZ_LOG_LEVEL LogVerbosity::Trace
#endif

// Helper macros to declare and define categories
#define DECLARE_LOG_CATEGORY_EXTERN(CategoryName, DefaultVerbosity)                                \
    extern struct FLogCategory##CategoryName : public LogCategoryBase {                            \
        FLogCategory##CategoryName()                                                               \
            : LogCategoryBase(#CategoryName, LogVerbosity::DefaultVerbosity) {}                    \
    } CategoryName;

#define DEFINE_LOG_CATEGORY_STATIC(CategoryName, DefaultVerbosity)                                 \
    struct FLogCategory##CategoryName : public LogCategoryBase {                                   \
        FLogCategory##CategoryName()                                                               \
            : LogCategoryBase(#CategoryName, LogVerbosity::DefaultVerbosity) {}                    \
    } CategoryName;

#define DEFINE_LOG_CATEGORY(CategoryName) FLogCategory##CategoryName CategoryName;

#if defined(__clang__)
    #define CZ_DISABLE_TAUTOLOGICAL_WARNING                                                        \
        _Pragma("clang diagnostic push")                                                           \
            _Pragma("clang diagnostic ignored \"-Wtautological-compare\"")
    #define CZ_ENABLE_TAUTOLOGICAL_WARNING _Pragma("clang diagnostic pop")
#else
    #define CZ_DISABLE_TAUTOLOGICAL_WARNING
    #define CZ_ENABLE_TAUTOLOGICAL_WARNING
#endif

#define CZ_LOG(Category, Verbosity, Format, ...)                                                   \
    do {                                                                                           \
        CZ_DISABLE_TAUTOLOGICAL_WARNING                                                            \
        if (static_cast<uint8_t>(LogVerbosity::Verbosity) <= static_cast<uint8_t>(CZ_LOG_LEVEL)) { \
            Logger::Get().Log(#Category, LogVerbosity::Verbosity,                                  \
                              fmt::format(fmt::runtime(Format), ##__VA_ARGS__));                   \
            if (LogVerbosity::Verbosity == LogVerbosity::Fatal) {                                  \
                CZ_DEBUGBREAK();                                                                   \
            }                                                                                      \
        }                                                                                          \
        CZ_ENABLE_TAUTOLOGICAL_WARNING                                                             \
    } while (0)

#define CZ_CORE_LOG(Verbosity, Format, ...) CZ_LOG(Core, Verbosity, Format, ##__VA_ARGS__)

#define CZ_RHI_LOG(Verbosity, Format, ...) CZ_LOG(RHI, Verbosity, Format, ##__VA_ARGS__)

#define CZ_BACKEND_LOG(Verbosity, Format, ...) CZ_LOG(Backend, Verbosity, Format, ##__VA_ARGS__)

#define CZ_RENDERCORE_LOG(Verbosity, Format, ...)                                                  \
    CZ_LOG(RenderCore, Verbosity, Format, ##__VA_ARGS__)

#define CZ_APP_LOG(Verbosity, Format, ...) CZ_LOG(App, Verbosity, Format, ##__VA_ARGS__)

#define CZ_EDITOR_LOG(Verbosity, Format, ...) CZ_LOG(Editor, Verbosity, Format, ##__VA_ARGS__)
