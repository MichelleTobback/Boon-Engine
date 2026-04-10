#pragma once
#include "Panels/EditorPanel.h"

#include <mutex>
#include <string>
#include <vector>

namespace BoonEditor
{
	struct ConsoleMessage
	{
		std::string Text;
		int Level = 0;
	};

	class ConsolePanel final : public EditorPanel
	{
	public:
		ConsolePanel(const std::string& name, EditorContext* pContext);
		virtual ~ConsolePanel();

		void AddMessage(const std::string& text, int level);
		void Clear();

	protected:
		virtual void OnRenderUI() override;

	private:

		std::vector<ConsoleMessage> m_Messages;
		std::mutex m_Mutex;
		bool m_AutoScroll{ true };
		bool m_ScrollToBottom{ false };

		std::shared_ptr<class EditorLogSink> m_pSink;
	};
}