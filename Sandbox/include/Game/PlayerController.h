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

    private:
        float m_MovementSpeed{ 3.f };
    };
}