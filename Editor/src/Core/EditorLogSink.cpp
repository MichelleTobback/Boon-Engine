#include "Core/EditorLogSink.h"
#include "Panels/ConsolePanel.h"

using namespace BoonEditor;

void EditorLogSink::Write(Boon::LogLevel level, const std::string& message)
{
	if (!m_pConsolePanel)
		return;

	int panelLevel{};

	switch (level)
	{
	case Boon::LogLevel::Info:
		panelLevel = 0;
		break;
	case Boon::LogLevel::Warning:
		panelLevel = 1;
		break;
	case Boon::LogLevel::Error:
		panelLevel = 2;
		break;
	}

	m_pConsolePanel->AddMessage(message, panelLevel);
}