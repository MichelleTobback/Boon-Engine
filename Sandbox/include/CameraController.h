#pragma once
#include <Core/Boon.h>

namespace Boon
{
    BCLASS(Name = "Camera controller", Category = "Components")
    struct CameraController
    {
    public:
        BCLASS_BODY()

       CameraController() = default;

        void Awake(GameObject gameObject);
        void Update(GameObject gameObject);

        void SetTarget(GameObject target) { m_Target = target; }

    private:
        BPROPERTY(Name = "follow speed", Category = "movement", RangeMin = 0.1, RangeMax = 20.f)
        float m_FollowSpeed{ 2.f };

        BPROPERTY(Name = "deadzone", Category = "movement", RangeMin = 0.1, RangeMax = 20.f)
        glm::vec2 m_Deadzone{ 3.f, 2.f };

        glm::vec3 m_TargetPosition{};

        BPROPERTY(Name = "target")
        BRef<TransformComponent> m_Target;

        glm::vec3 m_Velocity{};
    };
}