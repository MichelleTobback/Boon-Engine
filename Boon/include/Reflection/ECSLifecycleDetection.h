#pragma once
#include "Scene/GameObject.h"
#include <type_traits>
#include <utility>

namespace Boon
{
    // Awake(GameObject&)
    template<typename T, typename = void>
    struct has_awake : std::false_type {};

    template<typename T>
    struct has_awake<T, std::void_t<decltype(std::declval<T>().Awake(std::declval<GameObject&>()))>>
        : std::true_type {};

    // Update(GameObject&)
    template<typename T, typename = void>
    struct has_update : std::false_type {};

    template<typename T>
    struct has_update<T, std::void_t<decltype(std::declval<T>().Update(std::declval<GameObject&>()))>>
        : std::true_type {};

    // FixedUpdate(GameObject&)
    template<typename T, typename = void>
    struct has_fixedupdate : std::false_type {};

    template<typename T>
    struct has_fixedupdate<T, std::void_t<decltype(std::declval<T>().FixedUpdate(std::declval<GameObject&>()))>>
        : std::true_type {};

    // LateUpdate(GameObject&)
    template<typename T, typename = void>
    struct has_lateupdate : std::false_type {};

    template<typename T>
    struct has_lateupdate<T, std::void_t<decltype(std::declval<T>().LateUpdate(std::declval<GameObject&>()))>>
        : std::true_type {};

    // OnBeginOverlap(GameObject&, GameObject&)
    template<typename T, typename = void>
    struct has_onbeginoverlap : std::false_type {};

    template<typename T>
    struct has_onbeginoverlap<T, std::void_t<decltype(std::declval<T>().OnBeginOverlap(std::declval<GameObject&>(), std::declval<GameObject&>()))>>
        : std::true_type {};

    // OnEndOverlap(GameObject&, GameObject&)
    template<typename T, typename = void>
    struct has_onendoverlap : std::false_type {};

    template<typename T>
    struct has_onendoverlap<T, std::void_t<decltype(std::declval<T>().OnEndOverlap(std::declval<GameObject&>(), std::declval<GameObject&>()))>>
        : std::true_type {};
}
