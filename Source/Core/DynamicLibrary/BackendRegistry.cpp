#include <Core/DynamicLibrary/BackendRegistry.h>

#include <Core/Log/LogMacros.h>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogBackendRegistry, Info);

BackendRegistry& BackendRegistry::Get() {
    static BackendRegistry instance;
    return instance;
}

bool BackendRegistry::LoadBackend(const std::string& backendName, const std::string& libraryPath) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (m_Backends.find(backendName) != m_Backends.end()) {
        CZ_LOG(LogBackendRegistry, Warning, "Backend '{}' already loaded, skipping.", backendName);
        return true;
    }

    auto lib = std::make_unique<DynamicLibrary>(libraryPath);
    if (!lib->isValid()) {
        CZ_LOG(LogBackendRegistry, Error, "Failed to load backend library: {}", libraryPath);
        return false;
    }

    BackendData data;
    data.Library            = std::move(lib);
    m_Backends[backendName] = std::move(data);

    CZ_LOG(LogBackendRegistry, Info, "Backend '{}' loaded successfully.", backendName);
    return true;
}

void BackendRegistry::UnloadBackend(const std::string& backendName) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Backends.erase(backendName);
    CZ_LOG(LogBackendRegistry, Info, "Backend '{}' unloaded.", backendName);
}

} // namespace CZ