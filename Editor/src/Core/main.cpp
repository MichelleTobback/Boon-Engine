#include "Core/EditorState.h"

#include <Core/Application.h>

using namespace BoonEditor;

Boon::ENetDriverMode ParseDriverModeFromArgs(int argc, char** argv)
{
    // Default
    Boon::ENetDriverMode mode = Boon::ENetDriverMode::ListenServer;

    if (argc <= 1)
        return mode;

    std::string arg = argv[1];

    if (arg == "-server")
        mode = Boon::ENetDriverMode::DedicatedServer;
    else if (arg == "-listen")
        mode = Boon::ENetDriverMode::ListenServer;
    else if (arg == "-client")
        mode = Boon::ENetDriverMode::Client;

    return mode;
}

int main(int argc, char** argv)
{
	Boon::Application::AppDesc desc{};
	desc.name = "Boon Engine";
	desc.contentDir = "Content/";
	desc.windowDesc.name = "Boon Editor";
	desc.windowDesc.width = 800;
	desc.windowDesc.height = 600;
    desc.netDriverMode = ParseDriverModeFromArgs(argc, argv);
	Boon::Application app{ desc };
	app.Run(std::make_shared<EditorState>());
	return 0;
}