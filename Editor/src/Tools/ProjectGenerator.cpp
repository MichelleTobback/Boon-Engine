#include "Tools/ProjectGenerator.h"
#include "Project/ProjectLoader.h"
#include "tools/TemplateProcessor.h"

#include <fstream>

using namespace Boon;

namespace BoonEditor
{
	ProjectConfig ProjectGenerator::Generate(const ProjectGeneratorSettings& desc)
	{
		ProjectConfig proj{};

		InitializeProject(proj, desc);
		GenerateFilesFromTemplates(desc.TemplateFolder, proj, desc);

		return proj;
	}

	void ProjectGenerator::InitializeProject(ProjectConfig& project, const ProjectGeneratorSettings& desc)
	{
		ProjectLoader::ApplyDefaults(project);

		project.Name = desc.Name;
		project.Runtime.GameModule = desc.Name;
		project.Runtime.EnabledModules = { desc.Name };
		project.Runtime.ProjectRoot = desc.Location + "/" + desc.Name;

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

		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(templatesLoc))
		{
			ProcessTemplate(dirEntry, project, desc);
		}
	}

	void ProjectGenerator::ProcessTemplate(const std::filesystem::path& from, const ProjectConfig& project, const ProjectGeneratorSettings& desc)
	{
		std::ifstream file(from);
		if (!file.is_open())
			return;

		std::stringstream ss;

		ss << file.rdbuf();
		std::string content{ ss.str() };

		if (content.empty())
			return;

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

				return true;
			});
	}
}