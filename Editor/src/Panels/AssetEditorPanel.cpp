#include "Panels/AssetEditorPanel.h"
#include <Core/ServiceLocator.h>
#include <Asset/AssetLibrary.h>
#include <Scene/SceneManager.h>

BoonEditor::AssetEditorPanel::AssetEditorPanel(const std::string& name, DragDropRouter* pRouter, ViewportPanel* pViewport)
    : EditorPanel(name, pRouter), m_pViewport{pViewport}
{
	SceneManager& scenes = ServiceLocator::Get<SceneManager>();
	m_PreviewScene.Set(&scenes.CreateScene("asset preview"));

	m_Context.AddOnContextChangedCallback([this](AssetHandle handle)
		{
			for (auto& pEditor : m_pEditors)
			{
				if (pEditor->SetContext(handle))
				{
					m_pActiveEditor = pEditor.get();
					m_pViewport->SetContext(&m_PreviewScene);
					return;
				}
			}
			m_pActiveEditor = nullptr;
		});
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
    pEditor->m_PrevScene = &m_PreviewScene;
	m_pEditors.push_back(std::unique_ptr<AssetEditorBase>(pEditor));
}

void BoonEditor::AssetEditorPanel::OnRenderUI()
{
	if (!m_pActiveEditor)
		return;

	m_pActiveEditor->OnRender();
}
