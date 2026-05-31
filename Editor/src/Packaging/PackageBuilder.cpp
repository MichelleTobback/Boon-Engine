#include "Packaging/PackageBuilder.h"

#include "Packaging/PackageCodeGenerator.h"
#include <Process/ProcessRunner.h>

#include "BoonDebug/Logger.h"

#include <filesystem>
#include <fstream>

namespace BoonEditor
{
	static bool CopyDirectory(
		const std::filesystem::path& from,
		const std::filesystem::path& to)
	{
		if (from.empty() || !std::filesystem::exists(from))
			return false;

		std::filesystem::create_directories(to);

		std::error_code ec;
		std::filesystem::copy(
			from,
			to,
			std::filesystem::copy_options::recursive |
			std::filesystem::copy_options::overwrite_existing,
			ec);

		return !ec;
	}

	static std::filesystem::path NormalizeRepoRoot(std::filesystem::path root)
	{
		root = std::filesystem::absolute(root);

		if (std::filesystem::exists(root / "Boon" / "CMakeLists.txt"))
			return root;

		if (std::filesystem::exists(root / "CMakeLists.txt") &&
			root.filename() == "Boon")
		{
			return root.parent_path();
		}

		return root;
	}

	bool PackageBuilder::BuildPackage(
		const Boon::RuntimeConfig& config,
		const PackageModuleSet& modules,
		const PackageBuildSettings& settings,
		PackageLogCallback log,
		PackageProgressCallback progress)
	{
		auto Log = [&](const std::string& message)
			{
				if (log)
					log(message);

				BOON_LOG(message);
			};

		auto Error = [&](const std::string& message)
			{
				if (log)
					log("[Error] " + message);

				BOON_LOG_ERROR(message);
			};

		auto Step = [&](const std::string& message, float value)
			{
				if (progress)
					progress(message, value);

				Log("[Package] " + message);
			};

		const std::filesystem::path packageRoot =
			settings.OutputRoot / config.Window.Title;

		const std::filesystem::path generatedRoot =
			settings.GeneratedRoot.empty()
			? packageRoot / "Generated"
			: settings.GeneratedRoot;

		const std::filesystem::path repoRoot =
			NormalizeRepoRoot(settings.WorkspaceRoot);

		const std::filesystem::path packageTemplateDir = settings.TemplatePath;

		std::filesystem::create_directories(packageRoot);

		Step("Preparing package folder", 0.05f);

		const std::string platform = ToString(settings.BuildPlatform);

		if (settings.GenerateCode)
		{
			Step("Generating packaged runtime code", 0.20f);

			if (!PackageCodeGenerator::Generate(
				config,
				modules,
				settings,
				packageTemplateDir / "Common.btemplate",
				packageTemplateDir / (platform + ".btemplate"),
				packageRoot,
				generatedRoot,
				repoRoot))
			{
				Error("Failed to generate packaged runtime code.");
				return false;
			}
		}

		if (settings.CopyAssets)
		{
			Step("Copying assets", 0.40f);

			const std::filesystem::path gameAssetOut =
				packageRoot / "Assets" / "Game";

			const std::filesystem::path engineAssetOut =
				packageRoot / "Assets" / "Engine";

			if (!settings.GameAssetsSource.empty() &&
				!CopyDirectory(settings.GameAssetsSource, gameAssetOut))
			{
				Error("Failed to copy game assets.");
				return false;
			}

			if (!settings.EngineAssetsSource.empty() &&
				!CopyDirectory(settings.EngineAssetsSource, engineAssetOut))
			{
				Error("Failed to copy engine assets.");
				return false;
			}
		}

		{
			Step("Copying scenes", 0.48f);

			const std::filesystem::path scenesOut =
				packageRoot / "Assets" / "Game" / "Scenes";

			if (!settings.ProjectRoot.empty() &&
				!CopyDirectory(settings.ProjectRoot / "Assets" / "Scenes", scenesOut))
			{
				Error("Failed to copy scenes.");
				return false;
			}
		}

		Step("Running CMake build", 0.55f);

		const std::filesystem::path script = std::filesystem::absolute(packageRoot / ("Build" + platform + ".bat"));
		const std::string command = "cmd.exe /d /s /c \"call \"" + script.string() + "\"\"";

		ProcessResult result = ProcessRunner::Run(command, [&](const std::string& chunk)
			{
				if (log)
					log(chunk);

				BOON_LOG(chunk);
			});

		if (result.ExitCode != 0)
		{
			Error("Build failed.");
			return false;
		}

		Step("Package build complete", 1.0f);
		return true;
	}
}