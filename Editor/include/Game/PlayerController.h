#pragma once
#include <Core/Boon.h>

namespace Boon 
{
    BCLASS()
    struct PlayerController final
    {
    public:
        PlayerController() = default;

        void Update(GameObject gameObject);
        void FixedUpdate(GameObject gameObject);

        void OnBeginOverlap(GameObject gameObject, GameObject other);
        void OnEndOverlap(GameObject gameObject, GameObject other);

        enum class Direction
        {
            Left, Right
        };

    private:
        void CheckGrounded(GameObject gameObject);

        float m_MovementSpeed{ 10.f };
        glm::vec3 m_PrevPosition{};
        glm::vec2 m_MoveInput{ 0.0f };
        Direction m_Direction{ Direction::Left };
        bool m_IsGrounded{ false };
    };
}