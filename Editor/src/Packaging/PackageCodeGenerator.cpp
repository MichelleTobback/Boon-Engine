#include "Packaging/PackageCodeGenerator.h"

#include "Tools/TemplateProcessor.h"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

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

	static std::vector<std::string> BuildStaticModuleNames(
		const PackageModuleSet& modules,
		const std::string& gameProjectTarget)
	{
		std::vector<std::string> result;

		for (const PackageModule& module : modules.Modules)
		{
			const std::string name = SanitizeIdentifier(
				module.LinkTarget.empty() ? module.Name : module.LinkTarget);

			if (name.empty())
				continue;

			result.push_back(name);
		}

		result.push_back(gameProjectTarget);
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
		}
		return "Boon::ENetDriverMode::Standalone";
	}

	static std::string BuildPackageStaticModuleDeclarations(
		const std::vector<std::string>& moduleNames)
	{
		std::stringstream ss;

		for (const std::string& moduleName : moduleNames)
		{
			ss << "\tconst Boon::ModuleInfo* " << moduleName << "_GetModuleInfo();\n";
			ss << "\tBoon::ModuleRegistration " << moduleName
				<< "_RegisterModule(Boon::ModuleContext*);\n";
			ss << "\tvoid " << moduleName
				<< "_UnregisterModule(Boon::ModuleContext*, Boon::ModuleInstance*);\n\n";
		}

		return ss.str();
	}

	bool PackageCodeGenerator::Generate(
		const Boon::RuntimeConfig& config,
		const PackageModuleSet& modules,
		const std::filesystem::path& templatePath,
		const std::filesystem::path& outputDir)
	{
		std::ifstream file(templatePath);
		if (!file)
			return false;

		std::stringstream ss;
		ss << file.rdbuf();

		const std::string displayName = config.Window.Title.empty()
			? "Game"
			: config.Window.Title;

		const std::string targetName = SanitizeIdentifier(displayName);
		const std::string gameProjectTarget = SanitizeIdentifier(
			config.GameModule.empty() ? "Game" : config.GameModule);

		TemplateContext context;

		context.Set("GAME_NAME", EscapeCppString(displayName));
		context.Set("TARGET_NAME", targetName);
		context.Set("GAME_PROJECT_TARGET", gameProjectTarget);

		context.Set("STARTUP_SCENE", EscapeCppString(config.StartupScene));
		context.Set("GAME_MODULE", EscapeCppString(config.GameModule));

		context.Set("WINDOW_WIDTH", std::to_string(config.Window.Width));
		context.Set("WINDOW_HEIGHT", std::to_string(config.Window.Height));
		context.Set("WINDOW_RESIZABLE", BoolLiteral(config.Window.bResizable));
		context.Set("WINDOW_FULLSCREEN", BoolLiteral(config.Window.bFullscreen));

		context.Set("NET_NETMODE", NetMode(config.Network.NetMode));

		context.Set("MODULE_REGISTER_CODE", BuildModuleRegisterCode(modules));
		context.Set("MODULE_LINK_TARGETS", BuildModuleLinkTargets(modules));
		context.Set("MODULE_ON_ENTER", "");
		context.Set("MODULE_ON_UPDATE", "");
		context.Set("MODULE_ON_EXIT", "");
		context.Set("MODULE_FUNCTIONS", "");
		context.Set("MODULE_INCLUDES", "");

		context.Set("STATE_PRIVATE_DECLARATIONS", "");
		context.Set("STATE_PRIVATE_MEMBERS", "");

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
}