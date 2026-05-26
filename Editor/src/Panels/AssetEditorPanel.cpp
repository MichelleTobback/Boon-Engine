#include "Panels/AssetEditorPanel.h"

#include <Core/ServiceLocator.h>
#include <Asset/AssetLibrary.h>

BoonEditor::AssetEditorPanel::AssetEditorPanel(
    EditorContext* pContext,
    const std::string& name,
    ViewportPanel* pViewport)
    : EditorPanel(pContext, name),
    m_pViewport(pViewport)
{
    m_Context.AddOnContextChangedCallback(
        [this](AssetHandle handle)
        {
            for (auto& pEditor : m_pEditors)
            {
                if (pEditor->SetContext(handle))
                {
                    m_pActiveEditor = pEditor.get();

                    if (m_pViewport)
                        m_pViewport->SetCanvasRenderer(m_pActiveEditor);

                    return;
                }
            }

            if (m_pViewport)
                m_pViewport->ClearCanvasRenderer(m_pActiveEditor);

            m_pActiveEditor = nullptr;
        });
}

BoonEditor::AssetEditorPanel::~AssetEditorPanel()
{
    if (m_pViewport)
        m_pViewport->ClearCanvasRenderer(m_pActiveEditor);
}

void BoonEditor::AssetEditorPanel::Update()
{
    if (!m_pActiveEditor)
        return;

    m_pActiveEditor->Update();
}

void BoonEditor::AssetEditorPanel::RegisterEditor(AssetEditorBase* pEditor)
{
    pEditor->m_pViewport = m_pViewport;
    m_pEditors.push_back(std::unique_ptr<AssetEditorBase>(pEditor));
}

void BoonEditor::AssetEditorPanel::OnRenderUI()
{
    if (!m_pActiveEditor)
    {
        ImGui::TextDisabled("No asset editor active.");
        return;
    }

    m_pActiveEditor->RenderUI();
}