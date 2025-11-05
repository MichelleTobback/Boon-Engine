#pragma once
#include "Reflection/BClass.h"
#include "Reflection/ECSLifecycleDetection.h"
#include "Scene/GameObject.h"
#include <unordered_map>

namespace Boon
{
    struct ECSLifecycleSystem
    {
        Scene& scene;

        struct TypeCallbacks
        {
            std::function<void(GameObject&)> awake;
            std::function<void(Scene&)> update;
            std::function<void(Scene&)> fixedUpdate;
            std::function<void(Scene&)> lateUpdate;
        };

        std::unordered_map<std::type_index, TypeCallbacks> callbacks;

        ECSLifecycleSystem(Scene& s) : scene(s)
        {
            BClassRegistry::Get().ForEach([this](BClass& cls)
                {
                    if (cls.registerLifecycle)
                        cls.registerLifecycle(*this);
                });
        }

        template<typename T>
        void RegisterType()
        {
            TypeCallbacks cb;
            cb.awake = ECSLifecycleCallbacks<T>::awake;
            cb.update = ECSLifecycleCallbacks<T>::update;
            cb.fixedUpdate = ECSLifecycleCallbacks<T>::fixedUpdate;
            cb.lateUpdate = ECSLifecycleCallbacks<T>::lateUpdate;

            callbacks[typeid(T)] = cb;

            if constexpr (has_awake<T>::value)
            {
                scene.GetRegistry().on_construct<T>().connect(
                    [this](entt::registry& reg, entt::entity e)
                    {
                        auto it = callbacks.find(typeid(T));
                        if (it != callbacks.end() && it->second.awake)
                            it->second.awake(GameObject(e, &scene));
                    }
                );
            }
        }

        void UpdateAll() { for (auto& [_, cb] : callbacks) if (cb.update)      cb.update(scene); }
        void FixedUpdateAll() { for (auto& [_, cb] : callbacks) if (cb.fixedUpdate) cb.fixedUpdate(scene); }
        void LateUpdateAll() { for (auto& [_, cb] : callbacks) if (cb.lateUpdate)  cb.lateUpdate(scene); }
    };

    template<typename T>
    void RegisterLifecycleForType(ECSLifecycleSystem& system)
    {
        system.RegisterType<T>();
    }
}
