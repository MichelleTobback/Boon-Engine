#include "Packaging/PackageCodeGenerator.h"
#include "Packaging/PackageBuilder.h"

#include "Tools/TemplateProcessor.h"
#include "Packaging/PackageBuilder.h"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace Boon;

namespace BoonEditor
{
	static std::string BoolLiteral(bool value)
	{
		return value ? "true" : "false";
	}

	static std::string EscapeCppString(const std::string& value)
	{
		std::string result;

		for (char c : value)
		{
			switch (c)
			{
			case '\\': result += "\\\\"; break;
			case '"':  result += "\\\""; break;
			case '\n': result += "\\n"; break;
			case '\r': break;
			case '\t': result += "\\t"; break;
			default:   result += c; break;
			}
		}

		return result;
	}

	static std::string SanitizeIdentifier(std::string value)
	{
		for (char& c : value)
		{
			if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_')
				c = '_';
		}

		if (value.empty())
			value = "Game";

		if (std::isdigit(static_cast<unsigned char>(value[0])))
			value = "_" + value;

		return value;
	}

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

	static std::string BuildModuleRegisterCode(const PackageModuleSet& modules)
	{
		std::string result;

		for (const PackageModule& module : modules.Modules)
		{
			if (!module.RegisterCode.empty())
			{
				result += "\t\t// " + module.Name + "\n";
				result += "\t\t" + module.RegisterCode + "\n\n";
			}
		}

		return result;
	}

	static std::string BuildModuleLinkTargets(const PackageModuleSet& modules)
	{
		std::string result;

		for (const PackageModule& module : modules.Modules)
		{
			if (!module.LinkTarget.empty())
				result += "\t" + module.LinkTarget + "\n";
		}

		return result;
	}

	static std::string NetMode(ENetDriverMode mode)
	{
		switch (mode)
		{
		case ENetDriverMode::ListenServer:
			return "Boon::ENetDriverMode::ListenServer";

		case ENetDriverMode::DedicatedServer:
			return "Boon::ENetDriverMode::DedicatedServer";

		case ENetDriverMode::Client:
			return "Boon::ENetDriverMode::Client";

		default:
			return "Boon::ENetDriverMode::Standalone";
		}
	}

	static bool ProcessTemplate(
		const std::filesystem::path& templatePath,
		const std::filesystem::path& outputDir,
		const TemplateContext& context)
	{
		std::ifstream file(templatePath);
		if (!file)
			return false;

		std::stringstream ss;
		ss << file.rdbuf();

		std::filesystem::create_directories(outputDir);

		return TemplateProcessor::ProcessTemplateBlocks(
			ss.str(),
			"@@",
			context,
			[&](const std::filesystem::path& relativePath, std::string_view content)
			{
				const std::filesystem::path outPath = outputDir / relativePath;
				std::filesystem::create_directories(outPath.parent_path());

				std::ofstream out(outPath);
				if (!out)
					return false;

				out << content;
				return true;
			});
	}

	static TemplateContext BuildTemplateContext(
		const Boon::RuntimeConfig& config,
		const PackageModuleSet& modules,
		const PackageBuildSettings* settings,
		const std::filesystem::path& packageRoot,
		const std::filesystem::path& generatedRoot,
		const std::filesystem::path& repoRoot)
	{
		TemplateContext context;

		// ----------------------------
		// Runtime values
		// ----------------------------

		const std::string displayName =
			config.Window.Title.empty()
			? "Game"
			: config.Window.Title;

		const std::string targetName =
			SanitizeIdentifier(displayName);

		const std::string gameProjectTarget =
			SanitizeIdentifier(
				config.GameModule.empty()
				? "Game"
				: config.GameModule);

		context.Set("GAME_NAME", EscapeCppString(displayName));
		context.Set("TARGET_NAME", targetName);
		context.Set("GAME_PROJECT_TARGET", gameProjectTarget);

		context.Set("STARTUP_SCENE", EscapeCppString(config.StartupScene));
		context.Set("GAME_MODULE", EscapeCppString(config.GameModule));

		context.Set("WINDOW_WIDTH",
			std::to_string(config.Window.Width));

		context.Set("WINDOW_HEIGHT",
			std::to_string(config.Window.Height));

		context.Set("WINDOW_RESIZABLE",
			BoolLiteral(config.Window.bResizable));

		context.Set("WINDOW_FULLSCREEN",
			BoolLiteral(config.Window.bFullscreen));

		context.Set("NET_NETMODE",
			NetMode(config.Network.NetMode));

		context.Set("MODULE_REGISTER_CODE",
			BuildModuleRegisterCode(modules));

		context.Set("MODULE_LINK_TARGETS",
			BuildModuleLinkTargets(modules));

		context.Set("MODULE_ON_ENTER", "");
		context.Set("MODULE_ON_UPDATE", "");
		context.Set("MODULE_ON_EXIT", "");
		context.Set("MODULE_FUNCTIONS", "");
		context.Set("MODULE_INCLUDES", "");

		context.Set("STATE_PRIVATE_DECLARATIONS", "");
		context.Set("STATE_PRIVATE_MEMBERS", "");

		// ----------------------------
		// Build values
		// ----------------------------

		if (settings)
		{
			const std::filesystem::path buildDir =
				packageRoot / "Build";

			const std::filesystem::path engineRoot =
				repoRoot / "Boon";

			context.Set(
				"BUILD_PROFILE",
				settings->BuildProfileName.empty()
				? "Windows-Release"
				: settings->BuildProfileName);

			context.Set(
				"BUILD_CONFIGURATION",
				settings->BuildConfiguration.empty()
				? "Release"
				: settings->BuildConfiguration);

			context.Set("PACKAGE_ROOT", packageRoot.string());
			context.Set("GENERATED_ROOT", generatedRoot.string());
			context.Set("BUILD_DIR", buildDir.string());

			context.Set("BOON_REPO_ROOT", repoRoot.string());
			context.Set("BOON_SDK_ROOT", repoRoot.string());
			context.Set("BOON_ENGINE_ROOT", engineRoot.string());

			context.Set(
				"GAME_PROJECT_ROOT",
				settings->ProjectRoot.string());

			context.Set(
				"PACKAGE_ROOT_CMAKE",
				ToCMakePath(packageRoot));

			context.Set(
				"GENERATED_ROOT_CMAKE",
				ToCMakePath(generatedRoot));

			context.Set(
				"BUILD_DIR_CMAKE",
				ToCMakePath(buildDir));

			context.Set(
				"BOON_REPO_ROOT_CMAKE",
				ToCMakePath(repoRoot));

			context.Set(
				"BOON_SDK_ROOT_CMAKE",
				ToCMakePath(repoRoot));

			context.Set(
				"BOON_ENGINE_ROOT_CMAKE",
				ToCMakePath(engineRoot));

			context.Set(
				"GAME_PROJECT_ROOT_CMAKE",
				ToCMakePath(settings->ProjectRoot));
		}

		return context;
	}

	bool PackageCodeGenerator::Generate(
		const Boon::RuntimeConfig& config,
		const PackageModuleSet& modules,
		const PackageBuildSettings& settings,
		const std::filesystem::path& commonTemplatePath,
		const std::filesystem::path& platformTemplatePath,
		const std::filesystem::path& packageRoot,
		const std::filesystem::path& generatedRoot,
		const std::filesystem::path& repoRoot)
	{
		const TemplateContext context =
			BuildTemplateContext(
				config,
				modules,
				&settings,
				packageRoot,
				generatedRoot,
				repoRoot);

		if (!ProcessTemplate(
			commonTemplatePath,
			generatedRoot,
			context))
		{
			return false;
		}

		if (!ProcessTemplate(
			platformTemplatePath,
			generatedRoot,
			context))
		{
			return false;
		}

		return true;
	}
}