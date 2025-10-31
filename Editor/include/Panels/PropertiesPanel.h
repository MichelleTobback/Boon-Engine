#pragma once
#include "EditorPanel.h"
#include <memory>

namespace BoonEditor
{
	class PropertiesPanel final : public EditorPanel
	{
	public:
		PropertiesPanel(const std::string& name);
		virtual ~PropertiesPanel() = default;

		virtual void Update() override {}

	protected:
		virtual void OnRenderUI() override;
	};
}