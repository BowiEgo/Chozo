#pragma once

#include "CoreExport.h"
#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCore, Info);

#ifdef CZ_PLATFORM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
using LibHandle = HMODULE;
#else
    #include <dlfcn.h>
using LibHandle = void*;
#endif

// --- Module.h ---
class CORE_API CModule {
public:
    CModule()                          = default;
    CModule(const CModule&)            = delete;
    CModule& operator=(const CModule&) = delete;

    CModule(CModule&& other) noexcept; // [Note] Declaration only
    ~CModule();                        // [Note] Declaration only

    bool Load(const std::string& path); // [Note] Declaration only
    void Unload();                      // [Note] Declaration only

    template <typename Signature, typename... Args>
    auto Invoke(const std::string& name, Args&&... args) {
        using FuncPtr = Signature*;
        FuncPtr func  = reinterpret_cast<FuncPtr>(GetSymbol(name));

        if (!func) {
            CZ_LOG(LogCore, Error, "Failed to locate symbol: {0}", name);
            // [Note] Use a more robust way to return a "null" value
            using ReturnType = decltype(std::declval<FuncPtr>()(std::forward<Args>(args)...));
            return ReturnType{};
        }

        return func(std::forward<Args>(args)...);
    }

private:
    // [Note] Private method also needs to be in .cpp
    void* GetSymbol(const std::string& name) const;

private:
    LibHandle m_Handle = nullptr;
};