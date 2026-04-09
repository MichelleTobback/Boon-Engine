#pragma once
#include "EditorWidget.h"

#include <string>

namespace BoonEditor
{
    class EditorDialog : public EditorWidget
    {
    public:
        EditorDialog(const std::string& name, EditorContext* pContext);
        virtual ~EditorDialog() = default;

        void Open();
        void Close();
        bool IsOpen() const;
        void RenderUI();

    protected:
        virtual void RenderDialog() = 0;
        virtual void OnOpen() {}
        virtual void OnClose() {}

    protected:
        bool m_ShouldOpen = false;
        bool m_IsOpen = false;
    };
}