// RegisterBClass.h
#pragma once
#include "BClass.h"
#include "ECSLifecycleDetection.h"
#include "Component/ECSLifecycle.h"

namespace Boon
{
    template<typename T>
    void RegisterLifecycleForType(ECSLifecycleSystem& system);

    template<typename T>
    BClass* RegisterBClass()
    {
        static BClass cls(typeid(T).name(), typeid(T));

        if (has_awake<T>::value)       cls.SetFlag(BClass::Awake);
        if (has_update<T>::value)      cls.SetFlag(BClass::Update);
        if (has_fixedupdate<T>::value) cls.SetFlag(BClass::FixedUpdate);
        if (has_lateupdate<T>::value)  cls.SetFlag(BClass::LateUpdate);

        BClassRegistry::Get().Register(&cls);

        // Store function pointer for lifecycle registration
        cls.registerLifecycle = &RegisterLifecycleForType<T>;

        // Prepare static callback dispatchers
        ECSLifecycleCallbacks<T>::Init();

        return &cls;
    }
}

#define REGISTER_BCLASS(T) \
    namespace { static Boon::BClass* T##_bclass = Boon::RegisterBClass<T>(); }
