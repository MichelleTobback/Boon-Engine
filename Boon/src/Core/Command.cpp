#include "Core/Command.h"

//======================================================
//					Action Command					  //
//======================================================

Boon::ActionCommand::ActionCommand(const std::function<void()>& fnAction)
	: m_FnAction{fnAction}
{
}

void Boon::ActionCommand::Execute()
{
	if (m_FnAction)
		m_FnAction();
}