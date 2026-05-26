#pragma once

namespace Boon
{
    struct EngineContext;

    class ISubsystem
    {
    public:
        virtual ~ISubsystem() = default;

        virtual void OnInit(EngineContext&) = 0;
        virtual void OnShutdown(EngineContext&) = 0;
    };
}