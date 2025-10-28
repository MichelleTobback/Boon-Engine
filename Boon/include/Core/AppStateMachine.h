#pragma once
#include "AppState.h"

#include <memory>

namespace Boon
{
	class AppStateMachine final
	{
	public:
		AppStateMachine();
		virtual ~AppStateMachine() = default;

		AppStateMachine(const AppStateMachine& other) = delete;
		AppStateMachine(AppStateMachine&& other) = delete;
		AppStateMachine& operator=(const AppStateMachine& other) = delete;
		AppStateMachine& operator=(AppStateMachine&& other) = delete;

		void PushState(const std::shared_ptr<AppState>& state);
		void Shutdown();

		void Update();

	private:
		std::shared_ptr<AppState> m_pState;
	};
}