#pragma once

#include "Panels/EditorDialog.h"

#include "Packaging/PackageBuilder.h"
#include "Packaging/PackageModule.h"

#include <filesystem>
#include <string>
#include <atomic>
#include <mutex>
#include <thread>

namespace BoonEditor
{
	class PackageBuildDialog final : public EditorDialog
	{
	public:
		PackageBuildDialog(EditorContext* pContext, const std::string& name);
		virtual ~PackageBuildDialog();

	protected:
		void RenderDialog() override;
		void OnOpen() override;

	private:
		void RenderGeneralSettings();
		void RenderActions();
		
		void RenderProgress();
		void RenderLog();

		void StartBuildPackage();
		void AppendLog(const std::string& text);
		void SetStep(const std::string& step, float progress);

	private:
		std::filesystem::path m_OutputRoot = "Build/Packages/Windows";
		std::filesystem::path m_TemplatePath = "Templates/PackagedRuntime.btemplate";
		std::filesystem::path m_GeneratedRoot;
		std::string m_BuildProfileName = "Windows-Release";

		bool m_CopyAssets = true;
		bool m_GenerateCode = true;


		std::string m_Status;

		std::string m_BuildLog;
		std::mutex m_LogMutex;
		std::mutex m_StateMutex;

		std::thread m_BuildThread;
		std::atomic_bool m_IsBuilding = false;
		std::atomic<float> m_Progress = 0.0f;

		std::string m_CurrentStep;
		bool m_RunBuild = true;
	};
}