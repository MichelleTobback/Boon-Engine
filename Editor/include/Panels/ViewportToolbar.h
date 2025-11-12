#pragma once
#include "Core/BoonEditor.h"
#include "EditorPanel.h"

#include <memory>
#include <glm/glm.hpp>

namespace Boon
{
	class Texture2D;
}

namespace BoonEditor
{
	enum class ViewportToolbarSetting
	{
		None = 0,
		Camera = 1,
		Visibility = 2
	};

	class ViewportToolbar final : public EditorPanel
	{
	public:
		ViewportToolbar(const std::string& name);
		virtual ~ViewportToolbar() = default;

		ViewportToolbar(const ViewportToolbar& other) = delete;
		ViewportToolbar(ViewportToolbar&& other) = delete;
		ViewportToolbar& operator=(const ViewportToolbar& other) = delete;
		ViewportToolbar& operator=(ViewportToolbar&& other) = delete;

		void OnRender(const glm::vec2& boundsMin, const glm::vec2& boundsMax);

		void BindOnPlayCallback(const std::function<void()>& fn);
		void BindOnPauseCallback(const std::function<void()>& fn);
		void BindOnStopCallback(const std::function<void()>& fn);

		inline void SetActiveSetting(ViewportToolbarSetting setting) { m_ActiveSetting = setting; }
		inline ViewportToolbarSetting GetActiveSetting() const { return m_ActiveSetting; }

	protected:
		virtual void OnRenderUI() override {}

	private:
		void OnPlay();
		void OnPause();
		void OnStop();

		std::function<void()> m_fnOnPlayCallback{ nullptr };
		std::function<void()> m_fnOnPauseCallback{ nullptr };
		std::function<void()> m_fnOnStopCallback{ nullptr };
		EditorPlayState m_PlayState{ EditorPlayState::Edit };
		ViewportToolbarSetting m_ActiveSetting{};

		std::shared_ptr<Texture2D> m_pPlayIcon;
		std::shared_ptr<Texture2D> m_pStopIcon;
		std::shared_ptr<Texture2D> m_pCameraIcon;
		std::shared_ptr<Texture2D> m_pVisibilityIcon;
	};
}