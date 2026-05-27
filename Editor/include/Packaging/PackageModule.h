#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace BoonEditor
{
	struct PackageModule
	{
		std::string Name;
		std::string Include;
		std::string LinkTarget;
		std::string RegisterCode;

		bool BuiltIn = false;

		std::vector<std::filesystem::path> RuntimeFiles;
	};

	struct PackageModuleSet
	{
		std::vector<PackageModule> Modules;
	};
}