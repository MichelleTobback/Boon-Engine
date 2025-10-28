#include "Core/AppStateMachine.h"
#include "Core/AppStateMachine.h"

namespace Boon
{
	class NullAppState final : public AppState
	{
	public:
		virtual void OnEnter() override {}
		virtual void OnUpdate() override {}
		virtual void OnExit() override {}
	};
}

Boon::AppStateMachine::AppStateMachine()
	: m_pState{ std::make_shared<NullAppState>() }
{

}

void Boon::AppStateMachine::PushState(const std::shared_ptr<AppState>& pState)
{
	if (m_pState)
		m_pState->OnExit();
	m_pState = pState;
	m_pState->OnEnter();
}

void Boon::AppStateMachine::Shutdown()
{
	m_pState->OnExit();
	m_pState = nullptr;
}

void Boon::AppStateMachine::Update()
{
	m_pState->OnUpdate();
}
