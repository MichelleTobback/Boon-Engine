#include "Game/PlayerController.h"

#include <Scene/SceneManager.h>

#include "Core/ServiceLocator.h"
#include "Core/Time.h"
#include "Input/Input.h"

#include <Component/Rigidbody2D.h>
#include <Component/SpriteAnimatorComponent.h>

#include <Networking/NetIdentity.h>
#include <Networking/NetScene.h>
#include <Networking/NetRPC.h>

#include "Reflection/BClass.h"

#include <iostream>

using namespace Boon;

void PlayerController::Awake(GameObject gameObject)
{
    m_Owner = gameObject;
}

void PlayerController::Update(GameObject gameObject)
{
    // Input only
    Input& input = ServiceLocator::Get<Input>();
    glm::vec2 movement{};

    // Horizontal movement
    if (input.IsKeyHeld(Key::A))
    {
        movement.x = -1.0f;
        m_Direction = Direction::Left;
        gameObject.GetTransform().SetLocalScale(-1.f, 1.f, 1.f);
    }
    else if (input.IsKeyHeld(Key::D))
    {
        movement.x = 1.0f;
        m_Direction = Direction::Right;
        gameObject.GetTransform().SetLocalScale(1.f, 1.f, 1.f);
    }

    Rigidbody2D& rb = gameObject.GetComponent<Rigidbody2D>();
    if (m_IsGrounded && input.IsKeyPressed(Key::Space))
    {
        BClassRegistry::Get().Find<Boon::PlayerController>()->InvokeByName(this, "Jump");
    }

    // Normalize
    if (glm::length(movement) >= 1.0f)
        Move(glm::normalize(movement));
    else if (glm::length(m_MoveInput) >= 1.0f)
        Move({});

    SpriteAnimatorComponent& anim = gameObject.GetComponent<SpriteAnimatorComponent>();
    anim.SetClip((std::abs(rb.GetVelocity().x) > 0.001f || rb.GetVelocity().y >= 0.01f) ? 2 : 1);

    CheckGrounded(gameObject);
}

void PlayerController::FixedUpdate(GameObject gameObject)
{
    if (!gameObject.HasComponent<Rigidbody2D>())
        return;

    Rigidbody2D& rb = gameObject.GetComponent<Rigidbody2D>();

    const float dt = Time::Get().GetFixedTimeStep();
    glm::vec2 currentVel = rb.GetVelocity();

    if (m_MoveInput != glm::vec2(0.0f))
    {
        glm::vec2 targetVel = m_MoveInput * m_MovementSpeed;
        glm::vec2 velocityChange = targetVel - currentVel;

        const float accel = 25.0f;
        glm::vec2 force = velocityChange * rb.GetMass() * accel * dt;
        rb.AddForce(force);
    }
    else
    {
        // Instant stop for responsive feel
        glm::vec2 stopVel = currentVel * 0.15f;
        if (glm::length(stopVel) < 0.05f)
            stopVel = { 0.0f, 0.0f };
        rb.SetVelocity({ stopVel.x, rb.GetVelocity().y});
    }
}

void PlayerController::OnBeginOverlap(GameObject gameObject, GameObject other)
{
    std::cout << "Begin overlap\n";
}

void PlayerController::OnEndOverlap(GameObject gameObject, GameObject other)
{
    std::cout << "End overlap\n";
}

void PlayerController::Jump_Server()
{
    Rigidbody2D& rb = m_Owner.GetComponent<Rigidbody2D>();
    rb.AddForce({ 0.0f, rb.GetMass() * m_JumpForce }, Rigidbody2D::ForceMode::Impulse);
}

void PlayerController::Jump()
{
    NetIdentity& ni = m_Owner.GetComponent<NetIdentity>();
    if (ni.IsAutonomousProxy() || ni.IsSimulatedProxy())
    {
        BClassID clsId = BClassRegistry::Get().Find<PlayerController>()->hash;
        ni.pScene->GetRPC()->CallServer(clsId, FNV1a32("Jump_Server"), m_Owner.GetUUID(), {});
        return;
    }

    Rigidbody2D& rb = m_Owner.GetComponent<Rigidbody2D>();
    rb.AddForce({ 0.0f, rb.GetMass() * m_JumpForce }, Rigidbody2D::ForceMode::Impulse);
}

void PlayerController::Move_Server(glm::vec2 dir)
{
    m_MoveInput = dir;
}

void PlayerController::Move(const glm::vec2& dir)
{
    NetIdentity& ni = m_Owner.GetComponent<NetIdentity>();
    if (ni.IsAutonomousProxy() || ni.IsSimulatedProxy())
    {
        BClassID clsId = BClassRegistry::Get().Find<PlayerController>()->hash;
        Variant var{};
        var.Set<glm::vec2>(dir);
        ni.pScene->GetRPC()->CallServer(clsId, FNV1a32("Move_Server"), m_Owner.GetUUID(), { var });
        return;
    }

    m_MoveInput = dir;
}

void PlayerController::CheckGrounded(GameObject gameObject)
{
    Ray2D ray{};
    ray.Origin = glm::vec2(gameObject.GetTransform().GetWorldPosition()) - glm::vec2(0.f, 1.f) * 0.4f;
    ray.Direction = glm::vec2(0.f, -1.f);
    ray.Distance = 0.2f;

    HitResult2D result{};
    m_IsGrounded = gameObject.GetScene()->Raycast2D(ray, result);
}