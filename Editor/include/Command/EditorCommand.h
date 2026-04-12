#pragma once
#include <Core/Command.h>

namespace BoonEditor
{
	class EditorCommand : public Boon::Command
	{
	public:
		virtual ~EditorCommand() override = default;
		virtual void Execute() override = 0;
		virtual void Undo() = 0;
	};
}