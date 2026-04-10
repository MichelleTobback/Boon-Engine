#pragma once
#include "BoonDebug/Logger.h"

namespace BoonEditor
{
	class ConsolePanel;

	class EditorLogSink final : public Boon::ILogSink
	{
	public:
		EditorLogSink(ConsolePanel* pConsolePanel)
			: m_pConsolePanel(pConsolePanel)
		{
		}

		virtual void Write(Boon::LogLevel level, const std::string& message) override;

	private:
		ConsolePanel* m_pConsolePanel{};
	};
}