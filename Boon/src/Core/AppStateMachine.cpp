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

void Boon::AppStateMachine::RequestStateChange(const std::shared_ptr<AppState>& pState)
{
	m_pRequestState = pState;
}

void Boon::AppStateMachine::PushState(const std::shared_ptr<AppState>& pState, EngineContext& ctx)
{
	if (m_pState)
		m_pState->Exit();
	m_pState = pState;
	m_pState->Enter(ctx);
}

void Boon::AppStateMachine::Shutdown()
{
	m_pState->Exit();
	m_pState = nullptr;
}

void Boon::AppStateMachine::Update()
{
	m_pState->Update();
}

void Boon::AppStateMachine::EndUpdate(EngineContext& ctx)
{
	if (m_pRequestState)
	{
		PushState(m_pRequestState, ctx);
		m_pRequestState = nullptr;
	}
}
