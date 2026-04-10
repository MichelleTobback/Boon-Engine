#include "Tools/ProjectGenerator.h"
#include "Project/ProjectLoader.h"
#include "tools/TemplateProcessor.h"

#include "BoonDebug/Logger.h"
#include <fstream>

#include <algorithm>
#include <future>

using namespace Boon;

namespace BoonEditor
{
	ProjectConfig ProjectGenerator::Generate(const ProjectGeneratorSettings& desc)
	{
		static std::future<void> sProjectBuildFuture;
		ProjectConfig proj{};

		InitializeProject(proj, desc);
		GenerateFilesFromTemplates(desc.TemplateFolder, proj, desc);

		sProjectBuildFuture = std::async(std::launch::async, [proj]()
			{
				std::string log{};
				Configure(proj, proj.Runtime.ProjectRoot / "build", log);

				if (!Build(proj.Runtime.ProjectRoot / "build", "Debug", log))
					BOON_LOG_ERROR("Build failed!");
				else
					BOON_LOG("Build complete");
			});

		return proj;
	}

	void ProjectGenerator::InitializeProject(ProjectConfig& project, const ProjectGeneratorSettings& desc)
	{
		BOON_LOG("Creating new Project ... ");

		ProjectLoader::ApplyDefaults(project);

		project.Name = desc.Name;
		project.Runtime.GameModule = desc.Name;
		project.Runtime.EnabledModules = { desc.Name };
		project.Runtime.ProjectRoot = std::filesystem::path(desc.Location) / desc.Name;

		project.Runtime.Window.Title = desc.Name + " Project";

		std::filesystem::path location = desc.Location + "/" + project.Name;

		std::filesystem::create_directories(location);
		std::filesystem::create_directories(location / "Assets");

		ProjectLoader::SaveToFile(location, project);
	}

	void ProjectGenerator::GenerateFilesFromTemplates(const std::filesystem::path& templatesLoc, const ProjectConfig& project, const ProjectGeneratorSettings& desc)
	{
		if (!std::filesystem::exists(templatesLoc))
			return;

		BOON_LOG("Generating project files ... ");

		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(templatesLoc))
		{
			ProcessTemplate(dirEntry, project, desc);
		}
	}

	void ProjectGenerator::ProcessTemplate(const std::filesystem::path& from, const ProjectConfig& project, const ProjectGeneratorSettings& desc)
	{
		std::ifstream file(from);
		if (!file.is_open())
		{
			BOON_LOG_WARN("Failed to open temlate : {}, file skipped", from.string());
			return;
		}

		std::stringstream ss;

		ss << file.rdbuf();
		std::string content{ ss.str() };

		if (content.empty())
		{
			BOON_LOG_WARN("Failed to open temlate : {}, file skipped", from.string());
			return;
		}

		std::string uppercaseName = desc.Name;
		std::transform(uppercaseName.begin(), uppercaseName.end(), uppercaseName.begin(), ::toupper);

		std::string lowercaseName = desc.Name;
		std::transform(lowercaseName.begin(), lowercaseName.end(), lowercaseName.begin(), ::tolower);

		TemplateContext context{};
		context.Set("PROJECT_NAME", desc.Name);
		context.Set("PROJECT_NAME_UPPER", uppercaseName);
		context.Set("PROJECT_NAME_LOWER", lowercaseName);
		context.Set("PROJECT_ROOT", project.Runtime.ProjectRoot.string());
		context.Set("BOON_ENGINE_VERSION", std::to_string(project.Version));
		TemplateProcessor::ProcessTemplateBlocks(content, "@@", context, [project](const std::filesystem::path& path, std::string_view c)->bool
			{
				std::filesystem::path loc = project.Runtime.ProjectRoot / path;
				std::filesystem::create_directories(loc.parent_path());

				std::ofstream f(loc, std::ios::out | std::ios::binary | std::ios::trunc);
				if (!f.is_open())
					return false;

				f.write(c.data(), static_cast<std::streamsize>(c.size()));

				BOON_LOG("Generated {}", loc.string());

				return true;
			});
	}
	bool ProjectGenerator::Configure(const ProjectConfig& project, const std::filesystem::path& buildDir, std::string& outLog)
	{
		BOON_LOG("Configuring project ...");

		std::string configureCmd =
			"cmake -S \"" + project.Runtime.ProjectRoot.string() + "\" -B \"" + buildDir.string() + "\"";

		if (std::system(configureCmd.c_str()) != 0)
			return false;

		return true;
	}
	bool ProjectGenerator::Build(const std::filesystem::path& buildDir, const std::string& config, std::string& outLog)
	{
		BOON_LOG("Building project ...");

		std::string buildCmd =
			"cmake --build \"" + buildDir.string() + "\" --config Debug";

		if (std::system(buildCmd.c_str()) != 0)
			return false;

		return true;
	}
}