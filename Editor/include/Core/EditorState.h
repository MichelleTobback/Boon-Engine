#pragma once
#include "Core/BoonEditor.h"
#include "Core/EditorContext.h"
#include "Core/ObjectContext.h"
#include "Panels/EditorPanel.h"
#include "UI/DragDropRouter.h"

#include "Networking/NetworkSettings.h"
#include <Networking/NetPacket.h>

#include <Core/AppState.h>
#include <Renderer/SceneRenderer.h>
#include <Scene/SceneManager.h>

#include <Event/Event.h>

#include <memory>

namespace Boon
{
	class BClassRegistry;
	class ModuleLibrary;
}

namespace BoonEditor
{
	class EditorRenderer;
	class EditorCamera;
	class EditorState final : public Boon::AppState
	{
	public:
		EditorState(const Boon::ProjectConfig& project);
		virtual ~EditorState();

		EditorState(const EditorState& other) = default;
		EditorState(EditorState&& other) = default;
		EditorState& operator=(const EditorState& other) = default;
		EditorState& operator=(EditorState&& other) = default;

		virtual void OnEnter() override;
		virtual void OnUpdate() override;
		virtual void OnExit() override;

	private:
		void OnRender();
		void RenderMenuBar();

		void OnBeginPlay();
		void OnStopPlay();

		void StartNetwork();
		void StopNetwork();

		std::unique_ptr<EditorRenderer> m_PRenderer;
		GameObjectContext m_SelectionContext{};
		SceneContext m_SceneContext{};
		AssetContext* m_pSelectedAsset{};
		Boon::Scene* m_pSelectedScene;

		EditorPlayState m_PlayState{ EditorPlayState::Edit };

		Boon::EventListenerID m_SceneChangedEvent{};
		Boon::EventListenerID m_StateChangedEvent{};

		DragDropRouter m_DragDrop{};
		EditorContext m_Context;

		std::unique_ptr<Boon::ModuleLibrary> m_pModuleLib;
	};
}