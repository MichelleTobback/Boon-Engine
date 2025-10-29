#pragma once
#include "Assert.h"

#include <memory>
#include <unordered_map>

namespace Boon
{
	class ServiceLocator final
	{
    public:
        static void Shutdown()
        {
            GetRegistry().clear();
        }

        template<typename T>
        static void Register(std::shared_ptr<T> service)
        {
            GetRegistry()[typeid(T).hash_code()] = service;
        }

        template<typename T>
        static T& Get()
        {
            auto it{ GetRegistry().find(typeid(T).hash_code()) };
            BN_ASSERT(it != GetRegistry().end(), "Service not registered!");
            return *std::static_pointer_cast<T>(it->second);
        }

        template<typename T>
        static void Reset()
        {
            GetRegistry().erase(typeid(T).hash_code());
        }

    private:
        static std::unordered_map<size_t, std::shared_ptr<void>>& GetRegistry()
        {
            static std::unordered_map<size_t, std::shared_ptr<void>> registry;
            return registry;
        }
	};
}