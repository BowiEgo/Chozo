#pragma once

#include "CoreTypes.h"

enum class ELogVerbosity : uint8 {
    Fatal,   // Critical error, app cannot continue
    Error,   // Runtime error that requires attention
    Warning, // Unusual behavior that isn't an error yet
    Info,    // Major milestones or state changes
    Debug,   // Internal logic flow and variable states
    Trace    // High-frequency diagnostic or step-by-step tracing
};

#ifdef CZ_DIST
    #define CZ_LOG_LEVEL ELogVerbosity::Info
#else
    #define CZ_LOG_LEVEL ELogVerbosity::Trace
#endif

/**
 * Base class for all log categories
 */
struct CORE_API FLogCategoryBase {
    FLogCategoryBase(const char *InName, ELogVerbosity InDefaultVerbosity)
        : Name(InName), DefaultVerbosity(InDefaultVerbosity) {}

    const char *Name;
    ELogVerbosity DefaultVerbosity;
};

// Helper macros to declare and define categories
#define DECLARE_LOG_CATEGORY_EXTERN(CategoryName, DefaultVerbosity)            \
    extern struct FLogCategory##CategoryName : public FLogCategoryBase {       \
        FLogCategory##CategoryName()                                           \
            : FLogCategoryBase(#CategoryName,                                  \
                               ELogVerbosity::DefaultVerbosity) {}             \
    } CategoryName;

#define DEFINE_LOG_CATEGORY_STATIC(CategoryName, DefaultVerbosity)             \
    struct FLogCategory##CategoryName : public FLogCategoryBase {              \
        FLogCategory##CategoryName()                                           \
            : FLogCategoryBase(#CategoryName,                                  \
                               ELogVerbosity::DefaultVerbosity) {}             \
    } CategoryName;

#define DEFINE_LOG_CATEGORY(CategoryName)                                      \
    FLogCategory##CategoryName CategoryName;
