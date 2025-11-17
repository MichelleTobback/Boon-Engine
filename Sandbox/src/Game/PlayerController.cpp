#include "Game/PlayerController.h"

#include <Scene/SceneManager.h>

#include "Core/ServiceLocator.h"
#include "Core/Time.h"
#include "Input/Input.h"

#include <Component/SpriteAnimatorComponent.h>

using namespace Boon;

void Boon::PlayerController::Update(GameObject gameObject)
{
    Input& input{ ServiceLocator::Get<Input>() };

    float dt = Time::Get().GetDeltaTime();

    if (input.IsKeyHeld(Key::W))
    {
        float speed = m_Direction == Direction::Right ? m_MovementSpeed : -m_MovementSpeed;
        gameObject.GetTransform().Translate(dt * speed * gameObject.GetTransform().GetUp());
    }
    if (input.IsKeyHeld(Key::S))
    {
        float speed = m_Direction == Direction::Right ? -m_MovementSpeed : m_MovementSpeed;
        gameObject.GetTransform().Translate(dt * speed * gameObject.GetTransform().GetUp());
    }
    if (input.IsKeyHeld(Key::A))
    {
        float speed = -m_MovementSpeed;
        gameObject.GetTransform().SetLocalScale(-1.f, 1.f, 1.f);
        gameObject.GetTransform().Translate(dt * speed * gameObject.GetTransform().GetRight());
        m_Direction = Direction::Left;
    }
    if (input.IsKeyHeld(Key::D))
    {
        float speed = m_MovementSpeed;
        gameObject.GetTransform().SetLocalScale(1.f, 1.f, 1.f);
        gameObject.GetTransform().Translate(dt * speed * gameObject.GetTransform().GetRight());
        m_Direction = Direction::Right;
    }

    if (glm::distance(gameObject.GetTransform().GetWorldPosition(), m_PrevPosition) > 0.01f)
    {
        gameObject.GetComponent<SpriteAnimatorComponent>().Clip = 2;
    }
    else
    {
        gameObject.GetComponent<SpriteAnimatorComponent>().Clip = 1;
    }

    m_PrevPosition = gameObject.GetTransform().GetWorldPosition();
}