#include <Core/DynamicLibrary/DynamicLibraryRegistry.hpp>

#include <Core/Log/LogMacros.hpp>
#include <Core/Platform/Platform.h>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogDynamicLibraryRegistry, Info);

DynamicLibraryRegistry& DynamicLibraryRegistry::Get() {
    static DynamicLibraryRegistry instance;
    return instance;
}

bool DynamicLibraryRegistry::LoadLib(const std::string& libName, const std::string& libraryPath) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (m_Libs.find(libName) != m_Libs.end()) {
        CZ_LOG(LogDynamicLibraryRegistry, Warning, "Lib '{}' already loaded, skipping.", libName);
        return true;
    }

    auto lib = std::make_unique<DynamicLibrary>(libraryPath);
    if (!lib->isValid()) {
        CZ_LOG(LogDynamicLibraryRegistry, Error, "Failed to load lib library: {}", libraryPath);
        return false;
    }

    LibData data;
    data.Library    = std::move(lib);
    m_Libs[libName] = std::move(data);

    CZ_LOG(LogDynamicLibraryRegistry, Info, "Lib '{}' loaded successfully.", libName);
    return true;
}

void DynamicLibraryRegistry::UnloadLib(const std::string& libName) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Libs.erase(libName);
    CZ_LOG(LogDynamicLibraryRegistry, Info, "Lib '{}' unloaded.", libName);
}

} // namespace CZ