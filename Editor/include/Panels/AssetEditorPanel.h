#pragma once

#include "Panels/EditorPanel.h"
#include "Core/BoonEditor.h"
#include "Panels/AssetEditor.h"
#include "Panels/ViewportPanel.h"

#include <vector>
#include <memory>

namespace BoonEditor
{
    class AssetEditorPanel final : public EditorPanel
    {
    public:
        AssetEditorPanel(EditorContext* pContext, const std::string& name, ViewportPanel* pViewport);
        virtual ~AssetEditorPanel();

        virtual void Update() override;

        inline AssetContext& GetContext() { return m_Context; }

        void RegisterEditor(AssetEditorBase* pEditor);

    protected:
        virtual void OnRenderUI() override;

    private:
        AssetContext m_Context{};

        std::vector<std::unique_ptr<AssetEditorBase>> m_pEditors;
        AssetEditorBase* m_pActiveEditor = nullptr;

        ViewportPanel* m_pViewport = nullptr;
    };
}