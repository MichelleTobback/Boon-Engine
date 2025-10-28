#pragma once
#include "Core/Window.h"

#include <string>
#include <memory>

namespace Boon
{
	class AppState;
	class AppStateMachine;
	class Application final
	{
	public:
		struct AppDesc
		{
			Window::WindowDesc windowDesc{};
			std::string name{};
			std::string contentDir{};
		};
		Application(const AppDesc& desc);
		~Application();

		Application(const Application& other) = delete;
		Application(Application&& other) = delete;
		Application& operator=(const Application& other) = delete;
		Application& operator=(Application&& other) = delete;

		void Run(std::shared_ptr<AppState>&& pLayer);

		inline Window& GetWindow() const { return *m_pWindow; }
		inline AppStateMachine* GetStateMachine() const { return m_pStateMachine.get(); }
		inline uint32_t GetCurrentFrameIndex() const { return m_CurrentFrameIndex; }

		static Application& Get() { return *s_pInstance; }

	private:
		AppDesc m_Desc;
		std::unique_ptr<AppStateMachine> m_pStateMachine;
		std::unique_ptr<Window> m_pWindow{ nullptr };
		uint32_t m_CurrentFrameIndex{};

		static Application* s_pInstance;
	};
}