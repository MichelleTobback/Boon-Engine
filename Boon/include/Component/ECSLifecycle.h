#pragma once
#include "Reflection/BClass.h"
#include "Reflection/ECSLifecycleDetection.h"
#include "Scene/Scene.h"
#include <unordered_map>

namespace Boon
{
    template<typename T>
    struct ECSLifecycleCallbacks
    {
        static std::function<void(GameObject&)> awake;
        static std::function<void(Scene&)> update;
        static std::function<void(Scene&)> fixedUpdate;
        static std::function<void(Scene&)> lateUpdate;

        static void Init()
        {
            if constexpr (has_awake<T>::value)
                awake = [](GameObject& obj) { obj.GetComponent<T>().Awake(obj); };

            if constexpr (has_update<T>::value)
                update = [](Scene& scene)
                {
                    auto view = scene.GetRegistry().view<T>();
                    for (auto entity : view)
                    {
                        auto& comp = scene.GetRegistry().get<T>(entity);
                        comp.Update(GameObject(entity, &scene));
                    }
                };

            if constexpr (has_fixedupdate<T>::value)
                fixedUpdate = [](Scene& scene)
                {
                    auto view = scene.GetRegistry().view<T>();
                    for (auto entity : view)
                    {
                        auto& comp = scene.GetRegistry().get<T>(entity);
                        comp.FixedUpdate(GameObject(entity, &scene));
                    }
                };

            if constexpr (has_lateupdate<T>::value)
                lateUpdate = [](Scene& scene)
                {
                    auto view = scene.GetRegistry().view<T>();
                    for (auto entity : view)
                    {
                        auto& comp = scene.GetRegistry().get<T>(entity);
                        comp.LateUpdate(GameObject(entity, &scene));
                    }
                };
        }
    };

    template<typename T> std::function<void(GameObject&)> ECSLifecycleCallbacks<T>::awake;
    template<typename T> std::function<void(Scene&)> ECSLifecycleCallbacks<T>::update;
    template<typename T> std::function<void(Scene&)> ECSLifecycleCallbacks<T>::fixedUpdate;
    template<typename T> std::function<void(Scene&)> ECSLifecycleCallbacks<T>::lateUpdate;


    struct ECSLifecycleSystem
    {
        Scene& scene;

        struct Callbacks
        {
            std::function<void(GameObject&)> awake;
            std::function<void(Scene&)> update;
            std::function<void(Scene&)> fixedUpdate;
            std::function<void(Scene&)> lateUpdate;
        };

        std::unordered_map<std::type_index, Callbacks> map;

        ECSLifecycleSystem(Scene& s) : scene(s)
        {
            BClassRegistry::Get().ForEach([this](BClass& cls)
                {
                    if (cls.registerLifecycle)
                        cls.registerLifecycle(*this);
                });

            BClassRegistry::Get().AddListener([this](BClass& cls)
                {
                    if (cls.registerLifecycle)
                        cls.registerLifecycle(*this);
                });
        }

        template<typename T>
        void RegisterType()
        {
            if (map.find(typeid(T)) != map.end())
                return;

            ECSLifecycleCallbacks<T>::Init();

            Callbacks cb;
            cb.awake = ECSLifecycleCallbacks<T>::awake;
            cb.update = ECSLifecycleCallbacks<T>::update;
            cb.fixedUpdate = ECSLifecycleCallbacks<T>::fixedUpdate;
            cb.lateUpdate = ECSLifecycleCallbacks<T>::lateUpdate;

            map.emplace(typeid(T), cb);

            if constexpr (has_awake<T>::value)
            {
                scene.GetRegistry().on_construct<T>().connect(
                    [this](entt::registry& reg, entt::entity e)
                    {
                        auto it = map.find(typeid(T));
                        if (it != map.end() && it->second.awake)
                            it->second.awake(GameObject(e, &scene));
                    }
                );
            }
        }

        void UpdateAll() { for (auto& [_, cb] : map) if (cb.update)      cb.update(scene); }
        void FixedUpdateAll() { for (auto& [_, cb] : map) if (cb.fixedUpdate) cb.fixedUpdate(scene); }
        void LateUpdateAll() { for (auto& [_, cb] : map) if (cb.lateUpdate)  cb.lateUpdate(scene); }
    };
}
