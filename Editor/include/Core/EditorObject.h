#pragma once

namespace BoonEditor
{
	class EditorContext;
	class EditorObject
	{
	public:
		EditorObject(EditorContext* ctx)
			: m_pContext{ ctx } { }
		virtual ~EditorObject() = default;
		virtual void Update() = 0;

		inline EditorContext& GetContext() { return *m_pContext; }

	private:
		EditorContext* m_pContext;
	};
}