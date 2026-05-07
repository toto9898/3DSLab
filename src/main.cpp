#include "Application.h"

int main(int argc, char *argv[])
{
    Debugger3DS::Rendering::Application app;
    app.SetupViewer();

    // If a file was passed on the command line, open it
    if (argc > 1)
        app.OpenScene(argv[1]);

    app.Run();
}
