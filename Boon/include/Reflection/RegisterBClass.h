#pragma once
#include "BClass.h"
#include "Component/ECSLifecycle.h"
#include "Scene/GameObject.h"
#include "Scene/SceneManager.h"
#include "Core/ServiceLocator.h"

namespace Boon
{
    class BClassRegistry;
    class NetRepRegistry;

    template<typename T>
    BClass* RegisterBClass(BClassRegistry& registry, std::string name)
    {
        static BClass cls(name, typeid(T));

        if constexpr (has_awake<T>::value) 
        { 
            cls.SetFlag(BClass::Awake); 
            cls.awake = [](GameObject& owner) { owner.GetComponent<T>().Awake(owner); };
        }
        if constexpr (has_update<T>::value)      cls.SetFlag(BClass::Update);
        if constexpr (has_fixedupdate<T>::value) cls.SetFlag(BClass::FixedUpdate);
        if constexpr (has_lateupdate<T>::value)  cls.SetFlag(BClass::LateUpdate);

        cls.registerLifecycle = +[](ECSLifecycleSystem& sys)
            {
                sys.RegisterType<T>();
            };

        cls.unregister = []()
            {
                auto scenes = ServiceLocator::Get<SceneManager>().GetLoadedScenes();
                for (Scene* scene : scenes)
                {
                    scene->GetECSLifecycleSystem().UnregisterType<T>();
                    scene->GetRegistry().clear<T>();
                    scene->GetRegistry().reset(entt::type_hash<T>::value());
                }
            };

        cls.createInstance = []() -> void* 
            {
                return new T();
            };
        cls.destroyInstance = +[](void* p) { delete (T*)p; };
        cls.copyInstance = [](void* pSrc, void* pDst) -> void
            {
                T& src = *static_cast<T*>(pSrc);
                T& dst = *static_cast<T*>(pDst);
                dst = src;
            };

        cls.addComponent = [](GameObject& go) -> void*
            {
                T& comp = go.template AddComponent<T>();
                return static_cast<void*>(&comp);
            };
        cls.getComponent = [](GameObject& go) -> void*
            {
                T& comp = go.template GetComponent<T>();
                return static_cast<void*>(&comp);
            };
        cls.hasComponent = [](GameObject& go) -> bool
            {
                return go.template HasComponent<T>();
            };
        cls.removeComponent = [](GameObject& go) -> void
            {
                go.template RemoveComponent<T>();
            };

        registry.Register(&cls);
        return &cls;
    }

    template<typename T>
    BClass* RegisterBClass(std::string name)
    {
        return RegisterBClass<T>(BClassRegistry::Get(), name);
    }
}
