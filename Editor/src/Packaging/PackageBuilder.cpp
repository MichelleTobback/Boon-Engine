#include "Packaging/PackageBuilder.h"

#include "Packaging/PackageCodeGenerator.h"
#include <Process/ProcessRunner.h>

#include "BoonDebug/Logger.h"

#include <filesystem>
#include <fstream>

namespace BoonEditor
{
	static std::string ToCMakePath(const std::filesystem::path& path)
	{
		std::string value = path.string();
		for (char& c : value)
		{
			if (c == '\\')
				c = '/';
		}
		return value;
	}

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

	static bool WriteBuildScript(
		const std::filesystem::path& packageRoot,
		const std::filesystem::path& generatedRoot,
		const std::filesystem::path& engineRoot,
		const std::filesystem::path& projectRoot)
	{
		std::ofstream file(packageRoot / "BuildWindows.bat");
		if (!file)
			return false;

		const std::filesystem::path buildDir = packageRoot / "Build";

		file
			<< "@echo off\n"
			<< "setlocal\n"
			<< "cmake -S \"" << generatedRoot.string() << "\" -B \"" << buildDir.string() << "\" "
			<< "-DBOON_ENGINE_ROOT:PATH=\"" << ToCMakePath(engineRoot) << "\" "
			<< "-DGAME_PROJECT_ROOT:PATH=\"" << ToCMakePath(projectRoot) << "\"\n"
			<< "if errorlevel 1 exit /b 1\n"
			<< "cmake --build \"" << buildDir.string() << "\" --config Release\n"
			<< "if errorlevel 1 exit /b 1\n"
			<< "endlocal\n";

		return true;
	}

	bool PackageBuilder::BuildWindowsPackage(
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

		std::filesystem::create_directories(packageRoot);

		Step("Preparing package folder", 0.05f);

		if (settings.GenerateCode)
		{
			Step("Generating packaged runtime code", 0.20f);

			if (!PackageCodeGenerator::Generate(
				config,
				modules,
				settings.TemplatePath,
				generatedRoot))
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

		// tempory copy scenes
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

		Step("Writing build script", 0.55f);

		if (!WriteBuildScript(
			packageRoot,
			generatedRoot,
			settings.WorkspaceRoot,
			settings.ProjectRoot))
		{
			Error("Failed to write build script.");
			return false;
		}

		Step("Running CMake build", 0.65f);

		const std::filesystem::path script = packageRoot / "BuildWindows.bat";

		ProcessResult result = ProcessRunner::Run(
			"\"" + script.string() + "\"",
			[&](const std::string& chunk)
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