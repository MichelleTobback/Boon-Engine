#pragma once
#include "Core/Window.h"
#include "Networking/NetAuthority.h"

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
         * @brief Descriptor used to configure an Application instance.
         *
         * Contains window parameters, application name, content directory and
         * network driver mode. Fill this struct and pass it to the
         * Application constructor to initialize the runtime.
         */
		struct AppDesc
		{
			Window::WindowDesc windowDesc{};
			std::string name{};
			std::string contentDir{};
			ENetDriverMode netDriverMode{};
		};

        /**
         * @brief Construct the engine application with the given descriptor.
         *
         * Initializes core subsystems but does not start the main loop. Use
         * Run() to start execution.
         * @param desc Application descriptor used for initialization.
         */
		Application(const AppDesc& desc);
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
		 * @return Const reference to the AppDesc.
		 */
		inline const AppDesc& GetDescriptor() const { return m_Desc; }

	private:
		AppDesc m_Desc;
		std::unique_ptr<AppStateMachine> m_pStateMachine;
		std::unique_ptr<Window> m_pWindow{ nullptr };
		std::unique_ptr<class ServiceRegistry> m_pServiceRegistry;
		std::unique_ptr<class NetRepRegistry> m_pNetRepRegistry;
		std::unique_ptr<class BClassRegistry> m_pClsRegistry;

		uint32_t m_CurrentFrameIndex{};

		static Application* s_pInstance;
	};
}