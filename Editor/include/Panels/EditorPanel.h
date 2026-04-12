#pragma once
#include "EditorWidget.h"
#include "UI/DragDropRouter.h"

#include <string>
#include <glm/glm.hpp>
#include <vector>

namespace BoonEditor
{
	enum class EditorPanelFlag
	{
		None = 0,
		Hovered = 1 < 1
	};

	class EditorPanel : public EditorWidget
	{
	public:
		EditorPanel(const std::string& name, EditorContext* pContext);
		virtual ~EditorPanel() = default;

		EditorPanel(const EditorPanel& other) = delete;
		EditorPanel(EditorPanel&& other) = delete;
		EditorPanel& operator=(const EditorPanel& other) = delete;
		EditorPanel& operator=(EditorPanel&& other) = delete;

		virtual void Update() override {};
		virtual void OnResize(uint32_t width, uint32_t height) {}

		// Override from EditorWidget. Children of EditorPanel should override OnRenderUI
		virtual void RenderUI() override;

		bool IsHovered() const;

	protected:
		virtual void OnRenderUI() = 0;

		EditorPanelFlag m_Flags;
		DragDropRouter* m_pRouter;
	};
}