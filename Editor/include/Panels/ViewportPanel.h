#pragma once
#include "EditorPanel.h"
#include "Core/EditorCamera.h"

#include <memory>

#include <glm/glm.hpp>

namespace Boon
{
	class Scene;
	class SceneRenderer;
}

namespace BoonEditor
{
	class ViewportPanel final : public EditorPanel
	{
	public:
		ViewportPanel(const std::string& name, Scene* pContext);
		virtual ~ViewportPanel();

		virtual void Update() override;

	protected:
		virtual void OnRenderUI() override;

	private:
		std::unique_ptr<SceneRenderer> m_pRenderer;
		EditorCamera m_Camera{0.f, 0.f};

		bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2];
	};
}