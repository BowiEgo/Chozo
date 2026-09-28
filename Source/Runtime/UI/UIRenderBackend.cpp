#include <Runtime/UI/UIRenderBackend.hpp>

#include <Core/DynamicLibrary/DynamicLibraryRegistry.hpp>
#include <Core/DynamicLibrary/ModuleNames.hpp>
#include <Core/Log/LogMacros.hpp>

namespace CZ {

UIRenderBackendPtr CreateUIRenderBackend(std::string& err) {
    // The factory is exported by the graphics backend module, which is already loaded by the
    // time a window and a graphics context exist (see GraphicsContext::Create).
    using CreateUIBackendFn = UIRenderBackendObj* (*)();

    auto createFn = DynamicLibraryRegistry::Get().GetFunction<CreateUIBackendFn>(
        Modules::GraphicsBackendName, "CreateUIBackend");

    if (!createFn) {
        err = "The loaded graphics backend does not provide a UI backend.";
        return nullptr;
    }

    UIRenderBackendObj* backend = createFn();
    if (!backend) {
        err = "The graphics backend failed to create its UI backend.";
        return nullptr;
    }

    return UIRenderBackendPtr(backend);
}

} // namespace CZ
