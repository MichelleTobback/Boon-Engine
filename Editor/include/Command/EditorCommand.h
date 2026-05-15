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

	class EditorActionCommand final : public EditorCommand
	{
	public:
		explicit EditorActionCommand(const std::function<void()>& fnAction, const std::function<void()>& fnUndo)
			: m_FnAction{fnAction}, m_FnUndo{fnUndo} { }
		virtual ~EditorActionCommand() override = default;

		virtual void Execute() override
		{
			if (m_FnAction)
				m_FnAction();
		}

		virtual void Undo() override
		{
			if (m_FnUndo)
				m_FnUndo();
		}

	private:
		std::function<void()> m_FnAction;
		std::function<void()> m_FnUndo;
	};
}