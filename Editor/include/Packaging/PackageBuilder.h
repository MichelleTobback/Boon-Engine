#pragma once

#include "Project/RuntimeConfig.h"
#include "Packaging/PackageModule.h"
#include "Packaging/PackageBuildSettings.h"

#include <filesystem>
#include <functional>

namespace BoonEditor
{
	class PackageBuilder final
	{
	public:
		using PackageLogCallback = std::function<void(const std::string&)>;
		using PackageProgressCallback = std::function<void(const std::string&, float)>;

		static bool BuildPackage(
			const Boon::RuntimeConfig& config,
			const PackageModuleSet& modules,
			const PackageBuildSettings& settings,
			PackageLogCallback log,
			PackageProgressCallback progress);
	};
}