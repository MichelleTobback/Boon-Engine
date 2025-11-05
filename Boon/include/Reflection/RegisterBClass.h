#pragma once
#include "BClass.h"
#include "Component/ECSLifecycle.h"

namespace Boon
{
    template<typename T>
    BClass* RegisterBClass()
    {
        static BClass cls(typeid(T).name(), typeid(T));

        if constexpr (has_awake<T>::value)       cls.SetFlag(BClass::Awake);
        if constexpr (has_update<T>::value)      cls.SetFlag(BClass::Update);
        if constexpr (has_fixedupdate<T>::value) cls.SetFlag(BClass::FixedUpdate);
        if constexpr (has_lateupdate<T>::value)  cls.SetFlag(BClass::LateUpdate);

        cls.registerLifecycle = +[](ECSLifecycleSystem& sys)
            {
                sys.RegisterType<T>();
            };

        cls.createInstance = +[]() -> void* { return new T(); };
        cls.destroyInstance = +[](void* p) { delete (T*)p; };
        cls.addComponent = +[](GameObject& obj) -> void* { return &obj.AddComponent<T>(); };

        BClassRegistry::Get().Register(&cls);
        return &cls;
    }
}

#define REGISTER_BCLASS(T) \
    extern "C" void Boon_RegisterClass_##T() { Boon::RegisterBClass<T>(); }

