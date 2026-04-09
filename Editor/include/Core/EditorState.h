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
	class NetConnection;
	class NetRepRegistry;
	class BClassRegistry;
}

using namespace Boon;

namespace BoonEditor
{
	class EditorRenderer;
	class EditorCamera;
	class EditorState final : public AppState
	{
	public:
		EditorState(const ProjectConfig& project);
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

		void OnBeginPlay();
		void OnStopPlay();

		void StartNetwork();
		void StopNetwork();

		void OnConnected(NetConnection* pConnection);
		void OnDisconnected(NetConnection* pConnection);
		void OnPacketReceived(NetConnection* pConnection, NetPacket& packet);

		Boon::NetworkSettings m_NetworkSettings;

		std::unique_ptr<EditorRenderer> m_PRenderer;
		GameObjectContext m_SelectionContext{};
		SceneContext m_SceneContext{};
		AssetContext* m_pSelectedAsset{};
		Scene* m_pSelectedScene;

		EditorPlayState m_PlayState{ EditorPlayState::Edit };

		EventListenerID m_SceneChangedEvent{};
		EventListenerID m_StateChangedEvent{};
		EventListenerID m_BindNetSceneEvent{};
		Delegate<void(Scene&)>::Handle m_BindNetSceneHandle;

		DragDropRouter m_DragDrop{};

		EditorContext m_Context{};
	};
}