#pragma once
#include "BClass.h"
#include "Scene/GameObject.h"
#include <functional>
#include <type_traits>

namespace Boon
{
    // SFINAE checks
    template<typename T, typename = void> struct has_awake : std::false_type {};
    template<typename T> struct has_awake<T, std::void_t<decltype(std::declval<T>().Awake(GameObject{}))>> : std::true_type {};

    template<typename T, typename = void> struct has_update : std::false_type {};
    template<typename T> struct has_update < T, std::void_t<decltype(std::declval<T>().Update(GameObject{})) >> : std::true_type {};

    template<typename T, typename = void> struct has_fixedupdate : std::false_type {};
    template<typename T> struct has_fixedupdate < T, std::void_t<decltype(std::declval<T>().FixedUpdate(GameObject{})) >> : std::true_type {};

    template<typename T, typename = void> struct has_lateupdate : std::false_type {};
    template<typename T> struct has_lateupdate < T, std::void_t<decltype(std::declval<T>().LateUpdate(GameObject{})) >> : std::true_type {};

    // Per-type static callbacks
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
                awake = [](GameObject& obj)
                { 
                    obj.GetComponent<T>().Awake(obj); 
                };
            if constexpr (has_update<T>::value)
                update = [](Scene& scene)
                {
                    for (auto [e, comp] : scene.GetRegistry().view<T>().each())
                        comp.Update(GameObject(e, &scene));
                };
            if constexpr (has_fixedupdate<T>::value)
                fixedUpdate = [](Scene& scene)
                {
                    for (auto [e, comp] : scene.GetRegistry().view<T>().each())
                        comp.FixedUpdate(GameObject(e, &scene));
                };
            if constexpr (has_lateupdate<T>::value)
                lateUpdate = [](Scene& scene)
                {
                    for (auto [e, comp] : scene.GetRegistry().view<T>().each())
                        comp.LateUpdate(GameObject(e, &scene));
                };
        }
    };

    template<typename T> std::function<void(GameObject&)> ECSLifecycleCallbacks<T>::awake;
    template<typename T> std::function<void(Scene&)> ECSLifecycleCallbacks<T>::update;
    template<typename T> std::function<void(Scene&)> ECSLifecycleCallbacks<T>::fixedUpdate;
    template<typename T> std::function<void(Scene&)> ECSLifecycleCallbacks<T>::lateUpdate;
}
