#include <Runtime/App/Application.h>

using namespace CZ;

int main(int argc, char** argv) {
    std::string err;

    Application app;
    ApplicationSpecification spec{};

    if (!app.Startup(spec, err)) {
        CZ_APP_LOG(Error, "{}", err);
    }

    while (true) {
        app.Run();

        if (app.ShouldClose()) break;
    }

    app.Shutdown();
}