#include <Core/DynamicLibrary/DynamicLibraryRegistry.hpp>
#include <Core/Memory/Memory.hpp>
#include <Core/Memory/MemoryTypes.hpp>
#include <Runtime/App/Application.hpp>
#include <Runtime/App/StartupHost.hpp>

using namespace CZ;

int main(int argc, char** argv) {
    std::string err;

    Layer* editorLayer;
    StartupHost editor;

    // Load libraries
    {
        auto& registry = CZ::DynamicLibraryRegistry::Get();

        if (!registry.LoadLib("Editor", "libCZEditor.dylib")) {
            err = "Cannot load Editor.";
            return 0;
        }

        auto createEditorFn = registry.GetFunction<StartupHost (*)()>("Editor", "CreateEditor");
        if (!createEditorFn) {
            CZ_LOG(LogUI, Error, "CreateEditor not found in editor module.");
            return 0;
        }

        editor = createEditorFn();
    }

    Application& app = Application::Get();
    ApplicationSpecification spec{};

    app.SetStartupHost(editor);

    if (!app.Startup(spec, err)) {
        CZ_APP_LOG(Error, "{}", err);
    }

    while (true) {
        app.Run();

        if (app.ShouldClose()) break;
    }

    app.Shutdown();
}