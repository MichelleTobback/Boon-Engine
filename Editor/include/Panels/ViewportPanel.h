#pragma once
#include "EditorPanel.h"
#include "Core/BoonEditor.h"
#include "Panels/ViewportToolbar.h"

#include "Core/EditorCamera.h"

#include "DebugRenderer/EditorViewportSettings.h"

#include <Scene/GameObject.h>

#include <memory>

#include <glm/glm.hpp>

namespace Boon
{
	class Scene;
	class SceneRenderer;
}

namespace BoonEditor
{
	class DebugRenderer;
	class ViewportPanel final : public EditorPanel
	{
	public:
		ViewportPanel(const std::string& name, DragDropRouter* pRouter, SceneContext* pContext, GameObjectContext* pSelectionContext);
		virtual ~ViewportPanel();

		virtual void Update() override;

		inline ViewportToolbar* GetToolbar() const { return m_pToolbar.get(); }

		inline EditorCamera& GetCamera() { return m_Camera; }

		inline EditorViewportSettings& GetSettings() { return m_Settings; }
		inline const EditorViewportSettings& GetSettings() const { return m_Settings; }

	protected:
		virtual void OnRenderUI() override;

	private:
		void CameraSettings(float posX, float posY, float maxWidth);
		void VisibilitySettings(float posX, float posY, float maxWidth);

		std::unique_ptr<SceneRenderer> m_pRenderer;
		EditorCamera m_Camera{0.f, 0.f};

		bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2];

		GameObject m_HoveredGameObject{};
		SceneContext* m_pContext;
		GameObjectContext* m_pSelectionContext{};
		std::unique_ptr<ViewportToolbar> m_pToolbar{};
		std::unique_ptr<DebugRenderer> m_pDebugRenderer{};

		EditorViewportSettings m_Settings{};
	};
}