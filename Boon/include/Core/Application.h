#pragma once
#include "Core/Window.h"
#include "Networking/NetAuthority.h"
#include "Project/RuntimeConfig.h"
#include <Core/EngineContext.h>
#include <string>
#include <memory>
#include <functional>

namespace Boon
{
	class AppState;
	class AppStateMachine;
	class Application final
	{
	public:
        /**
         * @brief Construct the engine application with the given descriptor.
         *
         * Initializes core subsystems but does not start the main loop. Use
         * Run() to start execution.
         * @param desc RuntimeConfig descriptor used for initialization.
         */
		Application(const RuntimeConfig& desc);
        /**
         * @brief Destroy the application and shut down subsystems.
         */
		~Application();

		Application(const Application& other) = delete;
		Application(Application&& other) = delete;
		Application& operator=(const Application& other) = delete;
		Application& operator=(Application&& other) = delete;

		/**
		 * @brief Start the application's main loop with the initial state.
		 *
		 * Transfers ownership of the provided AppState into the application's
		 * state machine and runs the main update loop until exit.
		 * @param pLayer Initial application state to push.
		 */
		void Run(std::shared_ptr<AppState>&& pLayer);

		/**
		 * @brief Access the engine's context.
		 * Contains access to the Window, AssetLibrary, SceneManager, ...
		 * @return Reference to engine context.
		 */
		inline const EngineContext& GetContext() const { return m_Context; }

		/**
		 * @brief Access the application's main window.
		 * @return Reference to the Window instance.
		 */
		inline Window& GetWindow() const { return *m_pWindow; }
		/**
		 * @brief Get the internal AppStateMachine.
		 * @return Pointer to the application's state machine.
		 */
		inline AppStateMachine* GetStateMachine() const { return m_pStateMachine.get(); }
		/**
		 * @brief Get the current frame index since application start.
		 * @return Frame index (incremented each frame).
		 */
		inline uint32_t GetCurrentFrameIndex() const { return m_CurrentFrameIndex; }

		/**
		 * @brief Retrieve the global Application instance.
		 *
		 * The application instance is created by the program entry point and is
		 * accessible globally via this accessor.
		 * @return Reference to the singleton Application.
		 */
		static Application& Get() { return *s_pInstance; }

		/**
		 * @brief Get the descriptor used to create this Application.
		 * @return Const reference to the RuntimeConfig.
		 */
		inline const RuntimeConfig& GetDescriptor() const { return m_Desc; }

	private:
		std::unique_ptr<AppStateMachine> m_pStateMachine;
		std::unique_ptr<class ServiceRegistry> m_pServiceRegistry;
		std::unique_ptr<class NetRepRegistry> m_pNetRepRegistry;
		std::unique_ptr<class BClassRegistry> m_pClsRegistry;

		RuntimeConfig m_Desc;
		EngineContext m_Context;

		std::unique_ptr<class Window> m_pWindow;
		std::unique_ptr<class Input> m_pInput;
		std::unique_ptr<class AssetLibrary> m_pAssets;
		std::unique_ptr<class SceneManager> m_pScenes;
		std::unique_ptr<class EventBus> m_pEventBus;
		std::unique_ptr<class Time> m_pTime;
		std::unique_ptr<class SubsystemRegistry> m_pSubsystems;

		uint32_t m_CurrentFrameIndex{};

		static Application* s_pInstance;
	};
}