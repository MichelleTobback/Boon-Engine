#pragma once
#include "Core/BoonEditor.h"
#include "Core/EditorContext.h"
#include "Panels/EditorPanel.h"
#include "UI/DragDropRouter.h"

#include "Networking/NetworkSettings.h"
#include <Networking/NetPacket.h>

#include <Core/AppState.h>
#include <Renderer/SceneRenderer.h>

#include <Event/Event.h>

#include <memory>
#include <type_traits>
#include <vector>

namespace Boon
{
	class NetConnection;
}

using namespace Boon;

namespace BoonEditor
{
	class EditorRenderer;
	class EditorCamera;
	class EditorState final : public AppState
	{
	public:
		EditorState();
		~EditorState();

		EditorState(const EditorState& other) = default;
		EditorState(EditorState&& other) = default;
		EditorState& operator=(const EditorState& other) = default;
		EditorState& operator=(EditorState&& other) = default;

		virtual void OnEnter() override;
		virtual void OnUpdate() override;
		virtual void OnExit() override;

		template <typename T, typename ...TArgs>
		T& CreateObject(TArgs&& ... args)
		{
			static_assert(std::is_base_of<EditorObject, T>::value, "T must derive from Object");

			auto pInstance = std::make_unique<T>(std::forward<TArgs>(args)...);
			T& ref = *pInstance;
			m_Objects.push_back(std::move(pInstance));
			return ref;
		}

		template <typename T, typename ...TArgs>
		T& CreatePanel(const std::string& name, TArgs&& ... args)
		{
			static_assert(std::is_base_of<EditorPanel, T>::value, "T must derive from Panel");

			T& ref = CreateObject<T>(name, std::forward<TArgs>(args)...);
			m_Panels[name] = &ref;
			return ref;
		}

		template <typename T>
		T& GetPanel(const std::string& name)
		{
			static_assert(std::is_base_of<EditorPanel, T>::value, "T must derive from Panel");

			return *dynamic_cast<T*>(m_Panels[name]);
		}

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
		Scene* m_pSelectedScene;

		std::vector<std::unique_ptr<EditorObject>> m_Objects;
		std::unordered_map<std::string, EditorPanel*> m_Panels;

		EditorPlayState m_PlayState{ EditorPlayState::Edit };

		EventListenerID m_SceneChangedEvent;
		EventListenerID m_StateChangedEvent;
		EventListenerID m_BindNetSceneEvent;

		DragDropRouter m_DragDrop{};
	};
}