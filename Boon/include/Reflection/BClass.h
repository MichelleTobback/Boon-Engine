#pragma once
#include <string>
#include <typeindex>
#include <unordered_map>
#include <cstdint>
#include <functional>
#include <vector>

namespace Boon
{
    struct ECSLifecycleSystem;
    class GameObject;

    struct BClass
    {
        std::string name;
        std::type_index type;

        using RegisterFunc = void(*)(ECSLifecycleSystem&);
        using CreateInstanceFn = void* (*)();
        using DestroyInstanceFn = void(*)(void*);
        using AddComponentFn = void* (*)(GameObject&);

        RegisterFunc registerLifecycle = nullptr;
        CreateInstanceFn createInstance = nullptr;
        DestroyInstanceFn destroyInstance = nullptr;
        AddComponentFn addComponent = nullptr;

        enum FunctionFlags : uint8_t
        {
            None = 0,
            Awake = 1 << 0,
            Update = 1 << 1,
            FixedUpdate = 1 << 2,
            LateUpdate = 1 << 3
        };

        uint8_t flags = None;

        BClass(const char* n, std::type_index t) : name(n), type(t) {}

        bool HasAwake() const { return flags & Awake; }
        bool HasUpdate() const { return flags & Update; }
        bool HasFixedUpdate() const { return flags & FixedUpdate; }
        bool HasLateUpdate() const { return flags & LateUpdate; }

        void SetFlag(FunctionFlags f) { flags |= f; }
    };

    struct BClassRegistry
    {
        static BClassRegistry& Get()
        {
            static BClassRegistry instance;
            return instance;
        }

        void Register(BClass* cls)
        {
            m_Classes[cls->type] = cls;
            for (auto& listener : m_Listeners)
                listener(*cls);
        }

        void AddListener(std::function<void(BClass&)> fn)
        {
            m_Listeners.push_back(std::move(fn));
        }

        BClass* Find(std::type_index t)
        {
            auto it = m_Classes.find(t);
            return (it != m_Classes.end()) ? it->second : nullptr;
        }

        template<typename Fn>
        void ForEach(Fn&& fn)
        {
            for (auto& [_, cls] : m_Classes)
                fn(*cls);
        }

    private:
        std::unordered_map<std::type_index, BClass*> m_Classes;
        std::vector<std::function<void(BClass&)>> m_Listeners;
    };
}
