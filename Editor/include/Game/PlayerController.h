#pragma once
#include <Core/Boon.h>

namespace Boon 
{
    BCLASS(Name="Player controller", Category="Components")
    struct PlayerController
    {
    public:
        BCLASS_BODY()

        PlayerController() = default;

        void Awake(GameObject gameObject);
        void Update(GameObject gameObject);
        void FixedUpdate(GameObject gameObject);

        void OnBeginOverlap(GameObject gameObject, GameObject other);
        void OnEndOverlap(GameObject gameObject, GameObject other);

        BFUNCTION(RPC="Server")
        void Jump_Server();
        BFUNCTION()
        void Jump();

        BFUNCTION(RPC = "Server")
        void Move_Server(glm::vec2 dir);
        void Move(const glm::vec2& dir);

        enum class Direction
        {
            Left, Right
        };

    private:
        void CheckGrounded(GameObject gameObject);

        BPROPERTY(Name="speed", Category = "movement", RangeMin = 0.1, RangeMax = 20.f)
        float m_MovementSpeed{ 2.f };

        BPROPERTY(Name = "jump force", Category="movement", RangeMin = 0.1, RangeMax = 20.f)
        float m_JumpForce{5.f};

        glm::vec3 m_PrevPosition{};
        glm::vec2 m_MoveInput{ 0.0f };
        Direction m_Direction{ Direction::Left };
        bool m_IsGrounded{ false };

        GameObject m_Owner;
    };
}