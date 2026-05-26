#pragma once
#include "Core/EditorObject.h"

#include <string>

namespace BoonEditor
{
	class EditorContext;
	class EditorWidget : public EditorObject
	{
	public:
		EditorWidget(EditorContext* pContext, const std::string& name)
			: EditorObject(pContext), m_Name{ name } { }

		virtual ~EditorWidget() = default;
		virtual void Update() override {}
		virtual void RenderUI() = 0;

		inline const std::string& GetName() const { return  m_Name; }

	protected:
		std::string m_Name;
	};
}