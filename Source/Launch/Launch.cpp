#include <Core/DynamicLibrary/DynamicLibraryRegistry.h>
#include <Core/Memory/Memory.h>
#include <Core/Memory/MemoryTypes.h>
#include <Runtime/App/Application.h>

using namespace CZ;

int main(int argc, char** argv) {
    std::string err;

    Layer* editorLayer;

    // Load libraries
    {
        auto& registry = CZ::DynamicLibraryRegistry::Get();

        if (!registry.LoadLib("Editor", "./libCZEditor.dylib")) {
            err = "Cannot load Editor.";
            return 0;
        }

        auto createEditorLayerFn =
            registry.GetFunction<Layer* (*)()>("Editor", "CreateEditorLayer");
        if (!createEditorLayerFn) {
            CZ_LOG(LogUI, Error, "CreateEditorLayer not found in editor module.");
            return 0;
        }

        editorLayer = createEditorLayerFn();
    }

    Application& app = Application::Get();
    ApplicationSpecification spec{};

    app.SetStartupLayer(editorLayer);

    if (!app.Startup(spec, err)) {
        CZ_APP_LOG(Error, "{}", err);
    }

    while (true) {
        app.Run();

        if (app.ShouldClose()) break;
    }

    app.Shutdown();
}