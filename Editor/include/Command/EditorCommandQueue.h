#include "Command/EditorCommand.h"

#include <memory>
#include <queue>
#include <deque>

namespace BoonEditor
{
	class EditorCommandQueue final
	{
	public:
		EditorCommandQueue() = default;
		~EditorCommandQueue() = default;

		template <typename TCmd, typename ...TArgs>
		void Push(TArgs&& ... args)
		{
			static_assert(std::is_base_of<Boon::Command, TCmd>::value, "T must derive from Boon::Command");

			m_CommandQueue.push(std::make_unique<TCmd>(std::forward<TArgs>(args)...));
		}

		void Execute();
		void RequestUndo();

	private:
		std::queue<std::unique_ptr<Boon::Command>> m_CommandQueue;
		std::deque<std::unique_ptr<EditorCommand>> m_History;
	};
}