#pragma once
#include <Core/DynamicLibrary/DynamicLibrary.h>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace CZ {

class DynamicLibraryRegistry {
public:
    using FuncPtr = void*;

    static DynamicLibraryRegistry& Get();

    bool LoadLib(const std::string& libName, const std::string& libraryPath);
    void UnloadLib(const std::string& libName);

    template <typename FuncType>
    FuncType GetFunction(const std::string& libName, const std::string& funcName) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        auto libIt = m_Libs.find(libName);
        if (libIt == m_Libs.end()) return nullptr;

        auto& lib = libIt->second;

        auto funcIt = lib.Functions.find(funcName);
        if (funcIt != lib.Functions.end()) {
            return reinterpret_cast<FuncType>(funcIt->second);
        }

        FuncType func = lib.Library->getFunction<FuncType>(funcName.c_str());
        if (func) {
            lib.Functions[funcName] = reinterpret_cast<void*>(func);
        }
        return func;
    }

private:
    DynamicLibraryRegistry() = default;

    struct LibData {
        std::unique_ptr<DynamicLibrary> Library;
        std::unordered_map<std::string, FuncPtr> Functions;
    };

    mutable std::mutex m_Mutex;
    std::unordered_map<std::string, LibData> m_Libs;
};

} // namespace CZ