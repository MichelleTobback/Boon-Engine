#pragma once
#include "Assert.h"

#include <memory>
#include <unordered_map>
#include <stack>

namespace Boon
{
    class ServiceRegistry
    {
    public:
        template<typename T>
        void Register(std::shared_ptr<T> service)
        {
            size_t key = typeid(T).hash_code();
            m_Registry[key] = service;
            m_Keys.push(key);
        }

        template<typename T>
        T& Get()
        {
            auto it = m_Registry.find(typeid(T).hash_code());
            BN_ASSERT(it != m_Registry.end(), "Service not registered!");
            return *std::static_pointer_cast<T>(it->second);
        }

        template<typename T>
        void Reset()
        {
            m_Registry.erase(typeid(T).hash_code());
        }

        void Shutdown()
        {
            while (!m_Keys.empty())
            {
                m_Registry[m_Keys.top()] = nullptr;
                m_Keys.pop();
            }
            m_Registry.clear();
        }

    private:
        std::unordered_map<size_t, std::shared_ptr<void>> m_Registry;
        std::stack<size_t> m_Keys;
    };

	class ServiceLocator final
	{
    public:
        static void Shutdown()
        {
            GetRegistry()->Shutdown();
        }

        template<typename T>
        static void Register(std::shared_ptr<T> service)
        {
            GetRegistry()->Register<T>(service);
        }

        template<typename T>
        static T& Get()
        {
            return GetRegistry()->Get<T>();
        }

        template<typename T>
        static void Reset()
        {
            GetRegistry()->Reset<T>();
        }

        static void SetRegistry(ServiceRegistry* registry)
        {
            s_Registry = registry;
        }

        static ServiceRegistry* GetRegistry()
        {
            return s_Registry;
        }

    private:
        static ServiceRegistry* s_Registry;
	};
}