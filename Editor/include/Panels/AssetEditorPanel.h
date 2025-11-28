#include "Panels/EditorPanel.h"
#include "Core/BoonEditor.h"
#include "Panels/AssetEditor.h"
#include "Panels/ViewportPanel.h"
#include <vector>

namespace BoonEditor
{
	class AssetEditorPanel final : public EditorPanel
	{
	public:
		AssetEditorPanel(const std::string& name, DragDropRouter* pRouter, ViewportPanel* pViewport);

		virtual void Update() override;

		inline AssetContext& GetContext() { return m_Context; }
		inline SceneContext& GetScene() { return m_PreviewScene; }

		void RegisterEditor(AssetEditorBase* pEditor);

	protected:
		virtual void OnRenderUI() override;

	private:
		AssetContext m_Context{};
		SceneContext m_PreviewScene{};
		std::vector<std::unique_ptr<AssetEditorBase>> m_pEditors;
		AssetEditorBase* m_pActiveEditor{nullptr};
		ViewportPanel* m_pViewport{nullptr};
	};
}