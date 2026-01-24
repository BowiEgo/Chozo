#pragma once

#include "CoreTypes.h"

namespace Chozo {

    enum class ELogVerbosity : uint8 {
        Fatal,      // Critical error, crash the app
        Error,      // Runtime error
        Warning,    // Potentially dangerous
        Info,       // Standard output
        Trace       // Debugging information
    };

    /**
     * Base class for all log categories
     */
    struct CORE_API FLogCategoryBase {
        FLogCategoryBase(const char* InName, ELogVerbosity InDefaultVerbosity)
            : Name(InName), DefaultVerbosity(InDefaultVerbosity) {}

        const char* Name;
        ELogVerbosity DefaultVerbosity;
    };
}

// Helper macros to declare and define categories
#define DECLARE_LOG_CATEGORY_EXTERN(CategoryName, DefaultVerbosity) \
    extern struct FLogCategory##CategoryName : public Chozo::FLogCategoryBase { \
        FLogCategory##CategoryName() : Chozo::FLogCategoryBase(#CategoryName, Chozo::ELogVerbosity::DefaultVerbosity) {} \
    } CategoryName;

#define DEFINE_LOG_CATEGORY_STATIC(CategoryName, DefaultVerbosity) \
    struct FLogCategory##CategoryName : public Chozo::FLogCategoryBase { \
        FLogCategory##CategoryName() : Chozo::FLogCategoryBase(#CategoryName, Chozo::ELogVerbosity::DefaultVerbosity) {} \
    } CategoryName;

#define DEFINE_LOG_CATEGORY(CategoryName) \
    FLogCategory##CategoryName CategoryName;
    