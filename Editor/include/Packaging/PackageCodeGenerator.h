#pragma once

#include "Packaging/PackageBuilder.h"

#include <Project/RuntimeConfig.h>

#include <filesystem>

namespace BoonEditor
{
	struct PackageModuleSet;

	class PackageCodeGenerator
	{
	public:
		static bool Generate(
			const Boon::RuntimeConfig& config,
			const PackageModuleSet& modules,
			const PackageBuildSettings& settings,
			const std::filesystem::path& commonTemplatePath,
			const std::filesystem::path& platformTemplatePath,
			const std::filesystem::path& packageRoot,
			const std::filesystem::path& generatedRoot,
			const std::filesystem::path& repoRoot);
	};
}