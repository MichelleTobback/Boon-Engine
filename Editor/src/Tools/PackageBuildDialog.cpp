#include "Tools/PackageBuildDialog.h"

#include "Project/ProjectConfig.h"
#include "Core/EditorContext.h"

#include <fstream>
#include <algorithm>
#include <nlohmann/json.hpp>

#include <UI/UI.h>
#include <UI/IconsFontAwesome7.h>

#include <imgui.h>

namespace BoonEditor
{
	PackageBuildDialog::PackageBuildDialog(EditorContext* pContext, const std::string& name)
		: EditorDialog(pContext, name)
	{
	}

	PackageBuildDialog::~PackageBuildDialog()
	{
		if (m_BuildThread.joinable())
			m_BuildThread.join();
	}

	void PackageBuildDialog::OnOpen()
	{
		m_Status.clear();

		{
			std::scoped_lock lock(m_LogMutex);
			m_BuildLog.clear();
		}

		m_Progress = 0.0f;
		m_CurrentStep.clear();

		RefreshBuildProfiles();
	}

	void PackageBuildDialog::RenderDialog()
	{
		ImGui::TextUnformatted("Package Game");
		ImGui::Separator();

		RenderGeneralSettings();


		std::string step;
		{
			std::scoped_lock lock(m_StateMutex);
			step = m_CurrentStep;
		}
		if (!step.empty())
		{
			RenderProgress();
			RenderLog();
		}
		RenderActions();
	}

	void PackageBuildDialog::RenderGeneralSettings()
	{
		std::string outputRoot = m_OutputRoot.string();
		if (UI::Field("Output Root", outputRoot))
			m_OutputRoot = outputRoot;

		std::string templatePath = m_TemplatePath.string();
		if (UI::Field("Template", templatePath))
			m_TemplatePath = templatePath;

		RenderBuildProfileDropdown();

		UI::Checkbox("Generate Code", m_GenerateCode);
		UI::Checkbox("Copy Assets", m_CopyAssets);
		UI::Checkbox("Run Build", m_RunBuild);
	}

	void PackageBuildDialog::RenderProgress()
	{
		ImGui::Spacing();
		ImGui::TextDisabled("Progress");
		ImGui::Separator();

		std::string step;
		{
			std::scoped_lock lock(m_StateMutex);
			step = m_CurrentStep;
		}

		ImGui::ProgressBar(
			m_Progress.load(),
			ImVec2(-1.0f, 0.0f),
			step.empty() ? nullptr : step.c_str());
	}

	void PackageBuildDialog::RenderLog()
	{
		ImGui::Spacing();
		ImGui::TextDisabled("Build Log");
		ImGui::Separator();

		std::string logCopy;
		{
			std::scoped_lock lock(m_LogMutex);
			logCopy = m_BuildLog;
		}

		ImGui::BeginChild(
			"##package_build_log",
			ImVec2(0.0f, 220.0f),
			true,
			ImGuiWindowFlags_HorizontalScrollbar);

		ImGui::TextUnformatted(logCopy.c_str());

		if (m_IsBuilding)
			ImGui::SetScrollHereY(1.0f);

		ImGui::EndChild();
	}

	void PackageBuildDialog::RenderActions()
	{
		ImGui::Spacing();
		ImGui::Separator();

		if (!m_Status.empty())
			ImGui::TextWrapped("%s", m_Status.c_str());

		const bool bIsBuilding = m_IsBuilding;
		if (bIsBuilding)
			ImGui::BeginDisabled();

		if (ImGui::Button(ICON_FA_BOX_ARCHIVE " Build", ImVec2(120.0f, 0.0f)))
			StartBuildPackage();

		if (bIsBuilding)
			ImGui::EndDisabled();

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120.0f, 0.0f)))
			Close();
	}

	void PackageBuildDialog::AppendLog(const std::string& text)
	{
		std::scoped_lock lock(m_LogMutex);
		m_BuildLog += text;
		if (!text.empty() && text.back() != '\n')
			m_BuildLog += '\n';
	}

	void PackageBuildDialog::SetStep(const std::string& step, float progress)
	{
		{
			std::scoped_lock lock(m_StateMutex);
			m_CurrentStep = step;
		}

		m_Progress = progress;
		AppendLog("[Package] " + step);
	}

	static bool ReadBuildProfile(
		const std::filesystem::path& projectRoot,
		const std::string& profileName,
		BuildPlatform& outPlatform,
		std::string& outConfiguration)
	{
		const std::filesystem::path rulesPath = projectRoot / "BuildRules.json";

		std::ifstream file(rulesPath);
		if (!file)
			return false;

		nlohmann::json json;
		file >> json;

		if (!json.contains("profiles"))
			return false;

		const auto& profiles = json["profiles"];

		if (!profiles.contains(profileName))
			return false;

		const auto& profile = profiles[profileName];

		const std::string platform = profile.value("platform", "Windows");
		const std::string configuration = profile.value("configuration", "Release");

		outPlatform = ToPlatform(platform);

		outConfiguration = configuration;
		return true;
	}

	void PackageBuildDialog::StartBuildPackage()
	{
		if (m_IsBuilding)
			return;

		if (m_BuildThread.joinable())
			m_BuildThread.join();

		EditorContext& ctx = GetContext();
		const Boon::ProjectConfig prj = ctx.GetCurrentProjectConfig();

		Boon::RuntimeConfig config = prj.Runtime;
		config.Window.bResizable = true;
		config.Window.bFullscreen = false;

		PackageModuleSet modules{};

		PackageBuildSettings settings{};
		settings.TemplatePath = m_TemplatePath.is_absolute()
			? m_TemplatePath
			: prj.Editor.EditorResourcesRoot / m_TemplatePath;

		settings.GeneratedRoot = m_GeneratedRoot;
		settings.CopyAssets = m_CopyAssets;
		settings.GenerateCode = m_GenerateCode;

		settings.BuildProfileName = m_BuildProfiles[m_SelectedBuildProfileIndex].Name;
		settings.BuildPlatform = m_BuildProfiles[m_SelectedBuildProfileIndex].Platform;
		settings.BuildConfiguration = m_BuildProfiles[m_SelectedBuildProfileIndex].Configuration;

		settings.GameAssetsSource = config.ProjectRoot / "generated" / "Assets" / "Game";
		settings.EngineAssetsSource = config.ProjectRoot / "generated" / "Assets" / "Engine";
		settings.ProjectRoot = config.ProjectRoot;
		settings.WorkspaceRoot = prj.Runtime.EngineRoot;
		if (settings.WorkspaceRoot.filename() == "Boon")
			settings.WorkspaceRoot = settings.WorkspaceRoot.parent_path();

		settings.OutputRoot = m_OutputRoot / ToString(settings.BuildPlatform) / settings.BuildConfiguration;

		m_IsBuilding = true;

		{
			std::scoped_lock lock(m_LogMutex);
			m_BuildLog.clear();
		}

		{
			std::scoped_lock lock(m_StateMutex);
			m_Status = "Building package...";
			m_CurrentStep = "Starting...";
		}

		m_Progress = 0.0f;

		m_BuildThread = std::thread(
			[this, config, modules, settings]()
			{
				SetStep("Starting package build", 0.02f);

				const bool success = PackageBuilder::BuildPackage(
					config,
					modules,
					settings,
					[this](const std::string& text)
					{
						AppendLog(text);
					},
					[this](const std::string& step, float progress)
					{
						SetStep(step, progress);
					});

				{
					std::scoped_lock lock(m_StateMutex);

					m_Status = success
						? "Package generated successfully."
						: "Package build failed.";

					m_CurrentStep = success ? "Done" : "Failed";
				}

				m_Progress = success ? 1.0f : m_Progress.load();
				m_IsBuilding = false;
			});
	}

	void PackageBuildDialog::RefreshBuildProfiles()
	{
		m_BuildProfiles.clear();
		m_SelectedBuildProfileIndex = 0;

		EditorContext& ctx = GetContext();
		const Boon::ProjectConfig prj = ctx.GetCurrentProjectConfig();

		const std::filesystem::path rulesPath =
			prj.Runtime.ProjectRoot / "BuildProfiles.json";

		std::ifstream file(rulesPath);
		if (!file)
		{
			BuildProfile profile{};
			profile.Name = "Windows-Release";
			profile.Platform = BuildPlatform::Windows;
			profile.Configuration = "Release";
			m_BuildProfiles.push_back(profile);
			return;
		}

		nlohmann::json json;
		file >> json;

		if (!json.contains("profiles") || !json["profiles"].is_object())
		{
			BuildProfile profile{};
			profile.Name = "Windows-Release";
			profile.Platform = BuildPlatform::Windows;
			profile.Configuration = "Release";
			m_BuildProfiles.push_back(profile);
			return;
		}

		const std::string defaultProfile =
			json.value("defaultProfile", "Windows-Release");

		for (auto it = json["profiles"].begin(); it != json["profiles"].end(); ++it)
		{
			BuildProfile profile{};
			profile.Name = it.key();
			profile.Platform = ToPlatform(it->at("platform"));
			profile.Configuration = it->at("configuration");
			m_BuildProfiles.push_back(profile);
		}

		if (m_BuildProfiles.empty())
		{
			BuildProfile profile{};
			profile.Name = "Windows-Release";
			profile.Platform = BuildPlatform::Windows;
			profile.Configuration = "Release";
			m_BuildProfiles.push_back(profile);
		}

		const std::string preferredProfile = defaultProfile;

		auto found = std::find_if(
			m_BuildProfiles.begin(),
			m_BuildProfiles.end(),
			[&](const BuildProfile& profile)
			{
				return profile.Name == preferredProfile;
			});

		if (found != m_BuildProfiles.end())
			m_SelectedBuildProfileIndex = int(std::distance(m_BuildProfiles.begin(), found));
		else
			m_SelectedBuildProfileIndex = 0;
	}

	void PackageBuildDialog::RenderBuildProfileDropdown()
	{
		std::vector<const char*> temp;
		temp.reserve(m_BuildProfiles.size());

		for (const auto& profile : m_BuildProfiles)
			temp.push_back(profile.Name.c_str());

		if (temp.empty())
		{
			ImGui::TextDisabled("No build profiles found.");
			return;
		}

		UI::Combo("Build Profile", m_SelectedBuildProfileIndex, temp.data(), static_cast<int>(m_BuildProfiles.size()));
	}
}