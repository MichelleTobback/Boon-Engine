#include "Core/SandboxState.h"

#include <Core/Application.h>

using namespace Sandbox;

Boon::ENetDriverMode ParseDriverModeFromArgs(int argc, char** argv)
{
    // Default
    Boon::ENetDriverMode mode = Boon::ENetDriverMode::Client;

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

std::string NetDriverModeName(ENetDriverMode mode)
{
    switch (mode)
    {
    case ENetDriverMode::Standalone:
        return "[Standalone]";

    case ENetDriverMode::DedicatedServer:
        return "[Server]";

    case ENetDriverMode::ListenServer:
        return "[Listen Server]";
    case ENetDriverMode::Client:
        return "[Client]";
    }
    return "";
}

int main(int argc, char** argv)
{
	Boon::Application::AppDesc desc{};
    desc.netDriverMode = ParseDriverModeFromArgs(argc, argv);
	desc.name = "Boon Engine";
	desc.contentDir = "Content/";
	desc.windowDesc.name = "Boon Sandbox " + NetDriverModeName(desc.netDriverMode);
	desc.windowDesc.width = 800;
	desc.windowDesc.height = 600;
	Boon::Application app{ desc };
	app.Run(std::make_shared<SandboxState>());
	return 0;
}