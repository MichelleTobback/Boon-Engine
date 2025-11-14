#pragma once
#include <Core/Boon.h>

namespace Boon 
{
    BCLASS(Category = "Components", Name = "Player controller")
    struct PlayerController
    {
    public:
        BCLASS_BODY()

        PlayerController() = default;

        void Update(GameObject gameObject);

        enum class Direction
        {
            Left, Right
        };

    private:
        float m_MovementSpeed{ 3.f };
        glm::vec3 m_PrevPosition{};
        Direction m_Direction{ Direction::Left };
    };
}