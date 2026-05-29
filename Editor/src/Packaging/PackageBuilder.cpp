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

	static bool WriteBuildScript(
		const std::filesystem::path& packageRoot,
		const std::filesystem::path& generatedRoot,
		const std::filesystem::path& repoRoot,
		const std::filesystem::path& projectRoot,
		const std::string& profileName,
		const std::string& configuration)
	{
		std::ofstream file(packageRoot / "BuildWindows.bat");
		if (!file)
			return false;

		const std::filesystem::path buildDir = packageRoot / "Build";
		const std::string profile = profileName.empty() ? "Windows-Release" : profileName;
		const std::string config = configuration.empty() ? "Release" : configuration;

		file
			<< "@echo off\n"
			<< "setlocal\n"
			<< "\n"
			<< "echo [Setup] Finding Visual Studio build environment...\n"
			<< "set \"VS_DEV_CMD=\"\n"
			<< "set \"VSWHERE=%ProgramFiles(x86)%\\Microsoft Visual Studio\\Installer\\vswhere.exe\"\n"
			<< "\n"
			<< "if exist \"%VSWHERE%\" (\n"
			<< "  for /f \"usebackq tokens=*\" %%i in (`\"%VSWHERE%\" -latest -products * -property installationPath`) do (\n"
			<< "    if exist \"%%i\\Common7\\Tools\\VsDevCmd.bat\" (\n"
			<< "      set \"VS_DEV_CMD=%%i\\Common7\\Tools\\VsDevCmd.bat\"\n"
			<< "    )\n"
			<< "  )\n"
			<< ")\n"
			<< "\n"
			<< "if \"%VS_DEV_CMD%\"==\"\" (\n"
			<< "  if exist \"C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\Common7\\Tools\\VsDevCmd.bat\" (\n"
			<< "    set \"VS_DEV_CMD=C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\Common7\\Tools\\VsDevCmd.bat\"\n"
			<< "  )\n"
			<< ")\n"
			<< "\n"
			<< "if \"%VS_DEV_CMD%\"==\"\" (\n"
			<< "  if exist \"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Common7\\Tools\\VsDevCmd.bat\" (\n"
			<< "    set \"VS_DEV_CMD=C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Common7\\Tools\\VsDevCmd.bat\"\n"
			<< "  )\n"
			<< ")\n"
			<< "\n"
			<< "if \"%VS_DEV_CMD%\"==\"\" (\n"
			<< "  if exist \"C:\\Program Files\\Microsoft Visual Studio\\2022\\BuildTools\\Common7\\Tools\\VsDevCmd.bat\" (\n"
			<< "    set \"VS_DEV_CMD=C:\\Program Files\\Microsoft Visual Studio\\2022\\BuildTools\\Common7\\Tools\\VsDevCmd.bat\"\n"
			<< "  )\n"
			<< ")\n"
			<< "\n"
			<< "if \"%VS_DEV_CMD%\"==\"\" (\n"
			<< "  echo ERROR: Could not find VsDevCmd.bat.\n"
			<< "  exit /b 1\n"
			<< ")\n"
			<< "\n"
			<< "echo Using:\n"
			<< "echo   %VS_DEV_CMD%\n"
			<< "call \"%VS_DEV_CMD%\" -arch=x64 -host_arch=x64\n"
			<< "if errorlevel 1 exit /b 1\n"
			<< "\n"
			<< "where cl >nul 2>nul\n"
			<< "if errorlevel 1 (\n"
			<< "  echo ERROR: cl.exe not found after VsDevCmd setup.\n"
			<< "  exit /b 1\n"
			<< ")\n"
			<< "\n"
			<< "where rc >nul 2>nul\n"
			<< "if errorlevel 1 (\n"
			<< "  echo ERROR: rc.exe not found after VsDevCmd setup. Install the Windows SDK.\n"
			<< "  exit /b 1\n"
			<< ")\n"
			<< "\n"
			<< "where mt >nul 2>nul\n"
			<< "if errorlevel 1 (\n"
			<< "  echo ERROR: mt.exe not found after VsDevCmd setup. Install the Windows SDK.\n"
			<< "  exit /b 1\n"
			<< ")\n"
			<< "\n"
			<< "set \"PROFILE=" << profile << "\"\n"
			<< "set \"BOON_REPO_ROOT=" << repoRoot.string() << "\"\n"
			<< "set \"BOON_SDK_ROOT=" << repoRoot.string() << "\"\n"
			<< "set \"BOON_ENGINE_ROOT=" << (repoRoot / "Boon").string() << "\"\n"
			<< "set \"GAME_PROJECT_ROOT=" << projectRoot.string() << "\"\n"
			<< "\n"
			<< "set \"BOONBUILD_EXE=%BOON_REPO_ROOT%\\Tools\\BoonBuild.exe\"\n"
			<< "if not exist \"%BOONBUILD_EXE%\" set \"BOONBUILD_EXE=%BOON_REPO_ROOT%\\bin\\Release\\Tools\\BoonBuild.exe\"\n"
			<< "if not exist \"%BOONBUILD_EXE%\" set \"BOONBUILD_EXE=%BOON_REPO_ROOT%\\bin\\Debug\\Tools\\BoonBuild.exe\"\n"
			<< "\n"
			<< "if not exist \"%BOONBUILD_EXE%\" (\n"
			<< "  echo ERROR: BoonBuild.exe not found.\n"
			<< "  echo Tried:\n"
			<< "  echo   %BOON_REPO_ROOT%\\Tools\\BoonBuild.exe\n"
			<< "  echo   %BOON_REPO_ROOT%\\bin\\Release\\Tools\\BoonBuild.exe\n"
			<< "  echo   %BOON_REPO_ROOT%\\bin\\Debug\\Tools\\BoonBuild.exe\n"
			<< "  exit /b 1\n"
			<< ")\n"
			<< "\n"
			<< "echo SDK  : %BOON_SDK_ROOT%\n"
			<< "echo Engine: %BOON_ENGINE_ROOT%\n"
			<< "echo Tool : %BOONBUILD_EXE%\n"
			<< "echo.\n"
			<< "\n"
			<< "echo [1/3] Generating modular build files from BuildRules.json...\n"
			<< "\"%BOONBUILD_EXE%\" \"%GAME_PROJECT_ROOT%\" --profile \"%PROFILE%\"\n"
			<< "if errorlevel 1 exit /b 1\n"
			<< "\n"
			<< "echo [2/3] Configuring packaged runtime...\n"
			<< "cmake -S \"" << generatedRoot.string() << "\" -B \"" << buildDir.string() << "\" -G Ninja "
			<< "-DCMAKE_BUILD_TYPE=" << config << " "
			<< "-DBOON_REPO_ROOT:PATH=\"" << ToCMakePath(repoRoot) << "\" "
			<< "-DBOON_ENGINE_ROOT:PATH=\"" << ToCMakePath(repoRoot / "Boon") << "\" "
			<< "-DBOON_SDK_ROOT:PATH=\"" << ToCMakePath(repoRoot) << "\" "
			<< "-DGAME_PROJECT_ROOT:PATH=\"" << ToCMakePath(projectRoot) << "\"\n"
			<< "if errorlevel 1 exit /b 1\n"
			<< "\n"
			<< "echo [3/3] Building packaged runtime...\n"
			<< "cmake --build \"" << buildDir.string() << "\" --config " << config << "\n"
			<< "if errorlevel 1 exit /b 1\n"
			<< "\n"
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

		const std::filesystem::path repoRoot = NormalizeRepoRoot(settings.WorkspaceRoot);

		if (!WriteBuildScript(
			packageRoot,
			generatedRoot,
			repoRoot,
			settings.ProjectRoot,
			settings.BuildProfileName,
			settings.BuildConfiguration))
		{
			Error("Failed to write build script.");
			return false;
		}

		Step("Running CMake build", 0.65f);

		const std::filesystem::path script = packageRoot / "BuildWindows.bat";

		ProcessResult result = ProcessRunner::Run(
			"cmd /c \"" + script.string() + "\"",
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