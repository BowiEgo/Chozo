#pragma once

#include "Logger.h"

#include <fmt/core.h>

namespace CZ {

/**
 * Base class for all log categories
 */
struct FLogCategoryBase {
    FLogCategoryBase(const char* InName, ELogVerbosity InDefaultVerbosity)
        : Name(InName), DefaultVerbosity(InDefaultVerbosity) {}

    const char* Name;
    ELogVerbosity DefaultVerbosity;
};

} // namespace CZ

#ifdef CZ_DIST
    #define CZ_LOG_LEVEL ELogVerbosity::Info
#else
    #define CZ_LOG_LEVEL ELogVerbosity::Trace
#endif

// Helper macros to declare and define categories
#define DECLARE_LOG_CATEGORY_EXTERN(CategoryName, DefaultVerbosity)                                \
    extern struct FLogCategory##CategoryName : public FLogCategoryBase {                           \
        FLogCategory##CategoryName()                                                               \
            : FLogCategoryBase(#CategoryName, ELogVerbosity::DefaultVerbosity) {}                  \
    } CategoryName;

#define DEFINE_LOG_CATEGORY_STATIC(CategoryName, DefaultVerbosity)                                 \
    struct FLogCategory##CategoryName : public FLogCategoryBase {                                  \
        FLogCategory##CategoryName()                                                               \
            : FLogCategoryBase(#CategoryName, ELogVerbosity::DefaultVerbosity) {}                  \
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
        if (static_cast<uint8_t>(ELogVerbosity::Verbosity) <=                                      \
            static_cast<uint8_t>(CZ_LOG_LEVEL)) {                                                  \
            Logger::Get().Log(#Category, ELogVerbosity::Verbosity,                                 \
                              fmt::format(fmt::runtime(Format), ##__VA_ARGS__));                   \
            if (ELogVerbosity::Verbosity == ELogVerbosity::Fatal) {                                \
                CZ_DEBUGBREAK();                                                                   \
            }                                                                                      \
        }                                                                                          \
        CZ_ENABLE_TAUTOLOGICAL_WARNING                                                             \
    } while (0)

#define CZ_CORE_LOG(Verbosity, Format, ...) CZ_LOG(Core, Verbosity, Format, ##__VA_ARGS__)

#define CZ_APP_LOG(Verbosity, Format, ...) CZ_LOG(App, Verbosity, Format, ##__VA_ARGS__)