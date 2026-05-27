#pragma once

#include "Project/RuntimeConfig.h"
#include "Packaging/PackageModule.h"

#include <filesystem>

namespace BoonEditor
{
	class PackageCodeGenerator final
	{
	public:
		static bool Generate(
			const Boon::RuntimeConfig& config,
			const PackageModuleSet& modules,
			const std::filesystem::path& templatePath,
			const std::filesystem::path& outputDir);
	};
}