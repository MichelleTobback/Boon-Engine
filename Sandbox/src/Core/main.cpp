#include "Core/SandboxState.h"

#include <Core/Application.h>

using namespace Sandbox;

int main(int, char* [])
{
	Boon::Application::AppDesc desc{};
	desc.name = "Boon Engine";
	desc.contentDir = "Content/";
	desc.windowDesc.name = "Boon Sandbox";
	desc.windowDesc.width = 800;
	desc.windowDesc.height = 600;
	Boon::Application app{ desc };
	app.Run(std::make_shared<SandboxState>());
	return 0;
}