#pragma once

#include <Core/Platform/Platform.h>

namespace CZ {

/// File names of the modules that are loaded at runtime. Keeping them here means the loader
/// modules (launcher, graphics context) do not have to hard-code platform-specific extensions,
/// which matters as soon as more than one platform is supported.
namespace Modules {

#if defined(CZ_PLATFORM_WINDOWS)
inline constexpr const char* GraphicsBackendFile = "CZVulkan.dll";
inline constexpr const char* EditorFile          = "CZEditor.dll";
#elif defined(CZ_PLATFORM_MACOS)
inline constexpr const char* GraphicsBackendFile = "libCZVulkan.dylib";
inline constexpr const char* EditorFile          = "libCZEditor.dylib";
#else
inline constexpr const char* GraphicsBackendFile = "libCZVulkan.so";
inline constexpr const char* EditorFile          = "libCZEditor.so";
#endif

/// Key the backend is registered under in `DynamicLibraryRegistry`.
inline constexpr const char* GraphicsBackendName = "vulkan_backend";

} // namespace Modules

} // namespace CZ
