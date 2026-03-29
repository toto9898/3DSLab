#include "Application.h"
#include <iostream>

constexpr auto kMeshFilePath = R"(data\sample_model.3ds)";

int main(int argc, char *argv[])
{
    Debugger3DS::Application app;
    
    if (!app.LoadScene(kMeshFilePath)) {
        std::cerr << "Failed to load 3DS file" << std::endl;
        return -1;
    }
    
    app.SetupViewer();
    app.Run();
}
