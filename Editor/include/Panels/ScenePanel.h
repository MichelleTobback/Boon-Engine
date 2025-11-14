#pragma once
#include "EditorPanel.h"
#include "Core/BoonEditor.h"

#include <memory>

namespace Boon
{
	class Scene;
}

using namespace Boon;

namespace BoonEditor
{
	class ScenePanel final : public EditorPanel
	{
	public:
		ScenePanel(const std::string& name, SceneContext* pScene, GameObjectContext* pSelectedGameObject);
		virtual ~ScenePanel() = default;

		virtual void Update() override {}

	protected:
		virtual void OnRenderUI() override;

	private:
		void DrawGameObjectNode(GameObject gameObject);
		GameObject AddGameObjectPopup(GameObject parent);

		SceneContext* m_pSceneContext;
		GameObjectContext* m_pSelectionContext;
		UUID m_DraggedGameObject{ 0u };
	};
}