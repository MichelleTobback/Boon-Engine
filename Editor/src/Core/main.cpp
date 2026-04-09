#include "Core/EditorState.h"
#include "Project/ProjectLoader.h"
#include <Core/Application.h>

#include <iostream>

using namespace BoonEditor;
using namespace Boon;

ProjectConfig ProjectConfigFromArgs(int argc, char** argv)
{
    std::filesystem::path path = argc > 1 
        ? argv[1] 
        : "../../../Sandbox/Sandbox.bproj";

    return ProjectLoader::LoadFromFile(path).Value;
}

int main(int argc, char** argv)
{
    ProjectConfig config = ProjectConfigFromArgs(argc, argv);

	Boon::Application app{ config.Runtime };
    app.Run(std::make_shared<EditorState>(config));
	return 0;
}