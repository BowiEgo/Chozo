#pragma once
#include <Core/DynamicLibrary/DynamicLibrary.h>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace CZ {

class BackendRegistry {
public:
    using FuncPtr = void*;

    static BackendRegistry& Get();

    bool LoadBackend(const std::string& backendName, const std::string& libraryPath);
    void UnloadBackend(const std::string& backendName);

    template <typename FuncType>
    FuncType GetFunction(const std::string& backendName, const std::string& funcName) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        auto backendIt = m_Backends.find(backendName);
        if (backendIt == m_Backends.end()) return nullptr;

        auto& backend = backendIt->second;

        auto funcIt = backend.Functions.find(funcName);
        if (funcIt != backend.Functions.end()) {
            return reinterpret_cast<FuncType>(funcIt->second);
        }

        FuncType func = backend.Library->getFunction<FuncType>(funcName.c_str());
        if (func) {
            backend.Functions[funcName] = reinterpret_cast<void*>(func);
        }
        return func;
    }

private:
    BackendRegistry() = default;

    struct BackendData {
        std::unique_ptr<DynamicLibrary> Library;
        std::unordered_map<std::string, FuncPtr> Functions;
    };

    mutable std::mutex m_Mutex;
    std::unordered_map<std::string, BackendData> m_Backends;
};

} // namespace CZ