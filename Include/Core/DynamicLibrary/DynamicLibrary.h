#pragma once

#include <stdexcept>
#include <string>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

class DynamicLibrary {
public:
    explicit DynamicLibrary(const std::string& path) {
#ifdef _WIN32
        m_handle = LoadLibraryA(path.c_str());
#else
        m_handle = dlopen(path.c_str(), RTLD_NOW);
#endif
    }
    ~DynamicLibrary() {
        if (m_handle) {
#ifdef _WIN32
            FreeLibrary((HMODULE)m_handle);
#else
            dlclose(m_handle);
#endif
        }
    }

    template <typename T> T getFunction(const char* name) {
#ifdef _WIN32
        return reinterpret_cast<T>(GetProcAddress((HMODULE)m_handle, name));
#else
        return reinterpret_cast<T>(dlsym(m_handle, name));
#endif
    }

    void* getFunctionPtr(const char* name) const {
#ifdef _WIN32
        return reinterpret_cast<void*>(GetProcAddress((HMODULE)m_handle, name));
#else
        return dlsym(m_handle, name);
#endif
    }

    bool isValid() const { return m_handle != nullptr; }

private:
    void* m_handle = nullptr;
};