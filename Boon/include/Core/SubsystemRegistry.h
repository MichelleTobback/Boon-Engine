#pragma once
#include "Core/ISubsystem.h"

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <cassert>

namespace Boon
{
    struct EngineContext;

    class SubsystemRegistry
    {
    public:
        template<typename T, typename... Args>
        T& Register(Args&&... args)
        {
            static_assert(std::is_base_of_v<ISubsystem, T>);

            auto subsystem = std::make_unique<T>(std::forward<Args>(args)...);
            T& ref = *subsystem;

            m_Subsystems[typeid(T)] = std::move(subsystem);
            m_Order.push_back(typeid(T));

            return ref;
        }

        template<typename T>
        bool Has() const
        {
            return m_Subsystems.contains(typeid(T));
        }

        template<typename T>
        T& Get()
        {
            auto* subsystem = TryGet<T>();
            assert(subsystem && "Subsystem not registered");
            return *subsystem;
        }

        template<typename T>
        T* TryGet()
        {
            auto it = m_Subsystems.find(typeid(T));
            if (it == m_Subsystems.end())
                return nullptr;

            return static_cast<T*>(it->second.get());
        }

        void InitAll(EngineContext& ctx)
        {
            for (auto type : m_Order)
                m_Subsystems[type]->OnInit(ctx);
        }

        void ShutdownAll(EngineContext& ctx)
        {
            for (auto it = m_Order.rbegin(); it != m_Order.rend(); ++it)
                m_Subsystems[*it]->OnShutdown(ctx);

            m_Subsystems.clear();
            m_Order.clear();
        }

    private:
        std::unordered_map<std::type_index, std::unique_ptr<ISubsystem>> m_Subsystems;
        std::vector<std::type_index> m_Order;
    };
}