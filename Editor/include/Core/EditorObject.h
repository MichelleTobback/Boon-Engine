#pragma once

namespace BoonEditor
{
	class EditorObject
	{
	public:
		virtual ~EditorObject() = default;
		virtual void Update() = 0;
	};
}