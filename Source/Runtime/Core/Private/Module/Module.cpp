#include "Module.h"

// [Note] Implement logic here
CModule::CModule(CModule&& other) noexcept : m_Handle(other.m_Handle) { other.m_Handle = nullptr; }

CModule::~CModule() { Unload(); }

bool CModule::Load(const std::string& path) {
    if (m_Handle)
        return true;
#ifdef CZ_PLATFORM_WINDOWS
    m_Handle = LoadLibraryA(path.c_str());
#else
    m_Handle = dlopen(path.c_str(), RTLD_LAZY);
#endif
    return m_Handle != nullptr;
}

void CModule::Unload() {
    if (!m_Handle)
        return;
    // ... (FreeLibrary or dlclose)
    m_Handle = nullptr;
}

void* CModule::GetSymbol(const std::string& name) const {
    if (!m_Handle)
        return nullptr;
#ifdef CZ_PLATFORM_WINDOWS
    return reinterpret_cast<void*>(GetProcAddress(m_Handle, name.c_str()));
#else
    return dlsym(m_Handle, name.c_str());
#endif
}