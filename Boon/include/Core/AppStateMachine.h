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

		void RequestStateChange(const std::shared_ptr<AppState>& pState);
		void PushState(const std::shared_ptr<AppState>& pState);
		void Shutdown();

		void Update();
		void EndUpdate();

	private:
		std::shared_ptr<AppState> m_pState;
		std::shared_ptr<AppState> m_pRequestState{nullptr};
	};
}