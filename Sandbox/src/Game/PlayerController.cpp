#include "Game/PlayerController.h"

#include "Core/ServiceLocator.h"
#include "Core/Time.h"
#include "Input/Input.h"

using namespace Boon;

void Boon::PlayerController::Update(GameObject gameObject)
{
    Input& input{ ServiceLocator::Get<Input>() };
    float dt = Time::Get().GetDeltaTime();

    if (input.IsKeyHeld(Key::W))
        gameObject.GetTransform().Translate(dt * m_MovementSpeed * gameObject.GetTransform().GetUp());
    if (input.IsKeyHeld(Key::S))
        gameObject.GetTransform().Translate(-dt * m_MovementSpeed * gameObject.GetTransform().GetUp());
    if (input.IsKeyHeld(Key::A))
        gameObject.GetTransform().Translate(-dt * m_MovementSpeed * gameObject.GetTransform().GetRight());
    if (input.IsKeyHeld(Key::D))
        gameObject.GetTransform().Translate(dt * m_MovementSpeed * gameObject.GetTransform().GetRight());
}