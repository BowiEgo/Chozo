#include "Launch.h"

#include "Application.h"

int main(int argc, char** argv) {
    CApplication app;

    while (true) {
        app.Run();

        if (app.ShouldClose()) break;
    }

    app.Exit();

    return 0;
}