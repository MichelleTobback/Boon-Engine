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
        /**
         * @brief Register a service instance for type T.
         *
         * Stores a shared pointer to the provided service under the type's
         * hash. The registered service can later be retrieved by calling
         * ServiceRegistry::Get<T>().
         *
         * @tparam T Service type to register.
         * @param service Shared pointer to the service instance.
         */
        template<typename T>
        void Register(std::shared_ptr<T> service)
        {
            size_t key = typeid(T).hash_code();
            m_Registry[key] = service;
            m_Keys.push(key);
        }

        /**
         * @brief Retrieve a registered service instance by type.
         *
         * Will assert if no service of type T was registered. The returned
         * reference is the dereferenced stored shared_ptr and must not be
         * stored beyond the lifetime of the registry entry.
         *
         * @tparam T Service type to retrieve.
         * @return Reference to the registered service instance.
         */
        template<typename T>
        T& Get()
        {
            auto it = m_Registry.find(typeid(T).hash_code());
            BN_ASSERT(it != m_Registry.end(), "Service not registered!");
            return *std::static_pointer_cast<T>(it->second);
        }

        template<typename T>
        bool Has()
        {
            return m_Registry.contains(typeid(T).hash_code());
        }

        /**
         * @brief Unregister service of type T.
         *
         * Removes the service entry for the specified type if present.
         * @tparam T Service type to remove.
         */
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
        /**
         * @brief Global access point for services registered with the engine.
         *
         * ServiceLocator stores a pointer to a ServiceRegistry instance which
         * holds runtime service singletons. Use Register/Get/Reset to manage
         * services. Call Shutdown() to clear all registered services.
         */
        static void Shutdown()
        {
            GetRegistry()->Shutdown();
        }

        /**
         * @brief Register a service instance in the global registry.
         *
         * Convenience wrapper around ServiceRegistry::Register.
         * @tparam T Service type to register.
         * @param service Shared pointer to the service instance.
         */
        template<typename T>
        static void Register(std::shared_ptr<T> service)
        {
            GetRegistry()->Register<T>(service);
        }

        /**
         * @brief Retrieve a registered service from the global registry.
         *
         * Convenience wrapper around ServiceRegistry::Get. Asserts if the
         * requested service is not registered.
         * @tparam T Service type to retrieve.
         * @return Reference to the registered service instance.
         */
        template<typename T>
        static T& Get()
        {
            return GetRegistry()->Get<T>();
        }

        template<typename T>
        static bool Has()
        {
            if (!GetRegistry())
                return false;

            return GetRegistry()->Has<T>();
        }

        /**
         * @brief Unregister a service type from the global registry.
         *
         * Convenience wrapper around ServiceRegistry::Reset.
         * @tparam T Service type to unregister.
         */
        template<typename T>
        static void Reset()
        {
            GetRegistry()->Reset<T>();
        }

        /**
         * @brief Replace the internal ServiceRegistry instance.
         *
         * Allows embedding code (for example tests or tools) to provide a
         * custom registry implementation.
         * @param registry Pointer to the ServiceRegistry to use.
         */
        static void SetRegistry(ServiceRegistry* registry)
        {
            s_Registry = registry;
        }

        /**
         * @brief Access the underlying ServiceRegistry pointer.
         *
         * @return Pointer to the current ServiceRegistry.
         */
        static ServiceRegistry* GetRegistry()
        {
            return s_Registry;
        }

    private:
        static ServiceRegistry* s_Registry;
	};
}