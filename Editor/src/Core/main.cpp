#include "Core/EditorState.h"

#include <Core/Application.h>

using namespace BoonEditor;

int main(int, char* [])
{
	Boon::Application::AppDesc desc{};
	desc.name = "Boon Engine";
	desc.contentDir = "Content/";
	desc.windowDesc.name = "Boon Editor";
	desc.windowDesc.width = 800;
	desc.windowDesc.height = 600;
	Boon::Application app{ desc };
	app.Run(std::make_shared<EditorState>());
	return 0;
}