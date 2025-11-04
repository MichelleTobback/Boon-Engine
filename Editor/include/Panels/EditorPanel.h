#pragma once
#include "Core/EditorObject.h"

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

	class EditorPanel : public EditorObject
	{
	public:
		EditorPanel(const std::string& name);
		virtual ~EditorPanel() = default;

		EditorPanel(const EditorPanel& other) = delete;
		EditorPanel(EditorPanel&& other) = delete;
		EditorPanel& operator=(const EditorPanel& other) = delete;
		EditorPanel& operator=(EditorPanel&& other) = delete;

		virtual void Update() override {};
		virtual void OnResize(uint32_t width, uint32_t height) {}

		virtual void OnRender(){}

		void RenderUI();

		bool IsHovered() const;

	protected:
		virtual void OnRenderUI() = 0;

		bool RenderFloat3Control(const std::string& label, glm::vec3& vector, float resetValue = 0.0f, float columnWidth = 100.0f);

		inline const std::string& GetName() const { return  m_Name; }

	private:
		std::string m_Name;
		EditorPanelFlag m_Flags;
	};
}