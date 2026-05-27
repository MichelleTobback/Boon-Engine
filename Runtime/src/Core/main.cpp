#include <Core/Application.h>

#include "Core/RuntimeState.h"
#include "Project/ProjectLoader.h"

using namespace Runtime;

static Boon::ProjectConfig ProjectConfigFromArgs(int argc, char** argv)
{
	std::filesystem::path path = argc > 1
		? argv[1]
		: "../../..\\Sandbox\\Sandbox.bproj";

	return Boon::ProjectLoader::LoadFromFile(path).Value;
}

int main(int argc, char** argv)
{
	Boon::ProjectConfig config = ProjectConfigFromArgs(argc, argv);

	Boon::Application app{ config.Runtime };
	app.Run(std::make_shared<RuntimeState>());

	return 0;
}