#include "Command/EditorCommandQueue.h"

namespace BoonEditor
{
    class EditorUndoCommand final : public Boon::Command
    {
    public:
        explicit EditorUndoCommand(std::unique_ptr<EditorCommand> cmd)
            : m_pCommand(std::move(cmd))
        {
        }

        ~EditorUndoCommand() override = default;

        void Execute() override
        {
            if (m_pCommand)
                m_pCommand->Undo();
        }

    private:
        std::unique_ptr<EditorCommand> m_pCommand;
    };

    void EditorCommandQueue::Execute()
    {
        while (!m_CommandQueue.empty())
        {
            auto cmd = std::move(m_CommandQueue.front());
            m_CommandQueue.pop();

            cmd->Execute();

            if (auto* editorCmd = dynamic_cast<EditorCommand*>(cmd.get()))
            {
                auto raw = static_cast<EditorCommand*>(cmd.release());
                m_History.emplace_back(raw);
            }
        }
    }

    void EditorCommandQueue::RequestUndo()
    {
        if (m_History.empty())
            return;

        auto cmd = std::move(m_History.back());
        m_History.pop_back();

        Push<EditorUndoCommand>(std::move(cmd));
    }
}