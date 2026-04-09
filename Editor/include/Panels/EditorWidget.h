#pragma once
#include "Core/EditorObject.h"

#include <string>

namespace BoonEditor
{
	class EditorContext;
	class EditorWidget : public EditorObject
	{
	public:
		EditorWidget(const std::string& name, EditorContext* pContext)
			: EditorObject(), m_Name{ name }, m_pGameObjectContext{pContext} { }

		virtual ~EditorWidget() = default;
		virtual void Update() override {}
		virtual void RenderUI() = 0;

		inline EditorContext& GetContext() { return *m_pGameObjectContext; }
		inline const EditorContext& GetContext() const { return *m_pGameObjectContext; }

		inline const std::string& GetName() const { return  m_Name; }

	protected:
		std::string m_Name;
		EditorContext* m_pGameObjectContext;
	};
}