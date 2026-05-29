#pragma once

#include "Project/RuntimeConfig.h"
#include "Packaging/PackageModule.h"

#include <filesystem>
#include <functional>

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

		std::string BuildProfileName;
		std::string BuildConfiguration = "Release";

		bool CopyAssets = true;
		bool GenerateCode = true;
	};

	class PackageBuilder final
	{
	public:
		using PackageLogCallback = std::function<void(const std::string&)>;
		using PackageProgressCallback = std::function<void(const std::string&, float)>;

		static bool BuildWindowsPackage(
			const Boon::RuntimeConfig& config,
			const PackageModuleSet& modules,
			const PackageBuildSettings& settings,
			PackageLogCallback log,
			PackageProgressCallback progress);
	};
}