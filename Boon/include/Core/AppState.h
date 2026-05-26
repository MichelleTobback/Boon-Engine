#pragma once
#include <Core/EngineContext.h>

namespace Boon
{
    class AppState
    {
    public:
        virtual ~AppState() = default;

        void Enter(EngineContext& ctx)
        {
            m_pContext = &ctx;
            OnEnter();
        }

        void Update()
        {
            OnUpdate();
        }

        void Exit()
        {
            OnExit();
            m_pContext = nullptr;
        }

    protected:
        EngineContext& GetContext()
        {
            return *m_pContext;
        }

        virtual void OnEnter() = 0;
        virtual void OnUpdate() = 0;
        virtual void OnExit() = 0;

    private:
        EngineContext* m_pContext = nullptr;
    };
}