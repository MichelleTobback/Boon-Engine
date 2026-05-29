#include "Tools/PackageBuildDialog.h"

#include "Project/ProjectConfig.h"
#include "Core/EditorContext.h"

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

		std::scoped_lock lock(m_LogMutex);
		m_BuildLog.clear();

		m_Progress = 0.0f;
		m_CurrentStep.clear();
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

		UI::Field("Build Profile", m_BuildProfileName);

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

		// Modules are no longer selected manually here.
		// BoonBuild reads the active project's BuildRules.json and generates only
		// the engine/game modules requested by that project for this build profile.
		PackageModuleSet modules{};

		PackageBuildSettings settings{};
		settings.TemplatePath = m_TemplatePath.is_absolute()
			? m_TemplatePath
			: prj.Editor.EditorResourcesRoot / m_TemplatePath;

		settings.OutputRoot = m_OutputRoot;
		settings.GeneratedRoot = m_GeneratedRoot;
		settings.CopyAssets = m_CopyAssets;
		settings.GenerateCode = m_GenerateCode;
		settings.BuildProfileName = m_BuildProfileName;
		settings.BuildConfiguration =
			m_BuildProfileName.find("Debug") != std::string::npos
			? "Debug"
			: "Release";

		settings.GameAssetsSource = config.ProjectRoot / "generated" / "Assets" / "Game";
		settings.EngineAssetsSource = config.ProjectRoot / "generated" / "Assets" / "Engine";
		settings.ProjectRoot = config.ProjectRoot;
		settings.WorkspaceRoot = prj.Runtime.EngineRoot;
		if (settings.WorkspaceRoot.filename() == "Boon")
			settings.WorkspaceRoot = settings.WorkspaceRoot.parent_path();

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

				const bool success = PackageBuilder::BuildWindowsPackage(
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
}