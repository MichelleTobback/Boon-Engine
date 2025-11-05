// BClass.h
#pragma once
#include <string>
#include <typeindex>
#include <unordered_map>
#include <cstdint>

namespace Boon
{
    struct ECSLifecycleSystem;

    struct BClass
    {
        using RegisterFunc = void(*)(ECSLifecycleSystem&);

        std::string name;
        std::type_index type;
        RegisterFunc registerLifecycle = nullptr;

        enum FunctionFlags : uint8_t
        {
            None = 0,
            Awake = 1 << 0,
            Update = 1 << 1,
            FixedUpdate = 1 << 2,
            LateUpdate = 1 << 3
        };

        uint8_t flags = None;

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

        void Register(BClass* cls) { m_Classes[cls->type] = cls; }

        template<typename Fn>
        void ForEach(Fn&& fn)
        {
            for (auto& [_, cls] : m_Classes)
                fn(*cls);
        }

    private:
        std::unordered_map<std::type_index, BClass*> m_Classes;
    };
}
