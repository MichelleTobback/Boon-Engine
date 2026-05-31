#pragma once
#include <filesystem>
#include <BoonBuild/BuildPlatform.h>

namespace BoonEditor
{
	struct PackageBuildSettings
	{
		std::filesystem::path WorkspaceRoot;
		std::filesystem::path ProjectRoot;
		std::filesystem::path TemplatePath;
		std::filesystem::path OutputRoot;
		std::filesystem::path GeneratedRoot;

		std::filesystem::path GameAssetsSource;
		std::filesystem::path EngineAssetsSource;

		Boon::BuildPlatform BuildPlatform;
		std::string BuildProfileName = "Windows-Release";
		std::string BuildConfiguration = "Release";

		bool CopyAssets = true;
		bool GenerateCode = true;
	};
}