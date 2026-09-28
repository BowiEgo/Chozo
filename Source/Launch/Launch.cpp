#include <Core/DynamicLibrary/DynamicLibraryRegistry.hpp>
#include <Core/Memory/Memory.hpp>
#include <Core/Memory/MemoryTypes.hpp>
#include <Runtime/App/Application.hpp>
#include <Runtime/App/StartupHost.hpp>

using namespace CZ;

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    std::string err;
    StartupHost editor;

    // Load the editor module, which provides the startup host (and with it the main layer).
    {
        auto& registry = CZ::DynamicLibraryRegistry::Get();

        if (!registry.LoadLib("Editor", "libCZEditor.dylib")) {
            CZ_APP_LOG(Error, "Cannot load the Editor module.");
            return 1;
        }

        auto createEditorFn = registry.GetFunction<StartupHost (*)()>("Editor", "CreateEditor");
        if (!createEditorFn) {
            CZ_APP_LOG(Error, "CreateEditor not found in the editor module.");
            return 1;
        }

        editor = createEditorFn();
    }

    Application& app = Application::Get();
    ApplicationSpecification spec{};

    app.SetStartupHost(editor);

    if (!app.Startup(spec, err)) {
        CZ_APP_LOG(Error, "Startup failed: {}", err);
        return 1;
    }

    while (!app.ShouldClose()) {
        app.Run();
    }

    app.Shutdown();

    return 0;
}