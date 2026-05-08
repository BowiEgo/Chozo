#include "Application.h"

using namespace CZ;

int main(int argc, char** argv) {
    std::string err;

    Application app;
    ApplicationInfo info{};

    if (!app.Startup(info, err)) return 0;

    app.Run();
    app.Cleanup();
}