#include "PlayerController.h"

#include <Scene/SceneManager.h>

#include "Core/ServiceLocator.h"
#include "Core/Time.h"
#include "Input/Input.h"

#include <Component/Rigidbody2D.h>
#include <Component/SpriteAnimatorComponent.h>

#include <Networking/NetIdentity.h>
#include <Networking/NetScene.h>
#include <Networking/NetRPC.h>
#include <Networking/Components/NetRigidbody2D.h>

#include "Reflection/BClass.h"
#include "CameraController.h"

#include <Asset/Assets.h>

#include <iostream>

using namespace Boon;

void PlayerController::Awake(GameObject gameObject)
{
    m_Owner = gameObject;

    AssetLibrary& assetLib = Assets::Get();
    AssetRef<SpriteAtlasAsset> atlas = assetLib.Import<SpriteAtlasAsset>("game/Witch/Witch-combined.bsa");
    gameObject.GetOrAddComponent<SpriteRendererComponent>().SpriteAtlasHandle = atlas;

    if (gameObject.HasComponent<NetIdentity>() && !gameObject.GetComponent<NetIdentity>().IsOwner())
        return;

    gameObject.GetScene()->ForeachGameObjectWith<CameraController>([gameObject](GameObject obj)
        {
            obj.GetComponent<CameraController>().SetTarget(gameObject);
        });
}

PlayerController::Direction ClosestDirection(const glm::vec2& v)
{
    glm::vec2 n = glm::normalize(v);

    float up = glm::dot(n, glm::vec2(0, 1));
    float down = glm::dot(n, glm::vec2(0, -1));
    float right = glm::dot(n, glm::vec2(1, 0));
    float left = glm::dot(n, glm::vec2(-1, 0));

    float maxDot = up;
    PlayerController::Direction dir = PlayerController::Direction::Up;

    if (down > maxDot) { maxDot = down;  dir = PlayerController::Direction::Down; }
    if (right > maxDot) { maxDot = right; dir = PlayerController::Direction::Right; }
    if (left > maxDot) { maxDot = left;  dir = PlayerController::Direction::Left; }

    return dir;
}

void PlayerController::Update(GameObject gameObject)
{
    if (!gameObject.HasComponent<NetRigidbody2D>())
        return;

    NetRigidbody2D& rb = gameObject.GetComponent<NetRigidbody2D>();
    bool move = (glm::length2(rb.Velocity) > 0);
    SpriteAnimatorComponent& anim = gameObject.GetComponent<SpriteAnimatorComponent>();

    if (glm::length2(rb.Velocity) > 0.f)
        m_Direction = ClosestDirection(rb.Velocity);

    switch (m_Direction)
    {
    case Direction::Left:
        anim.SetClip(move ? 7 : 3);
        break;
    case Direction::Right:
        anim.SetClip(move ? 5 : 1);
        break;
    case Direction::Up:
        anim.SetClip(move ? 6 : 2);
        break;
    case Direction::Down:
        anim.SetClip(move ? 4 : 0);
        break;
    }

    CheckGrounded(gameObject);

    if (gameObject.HasComponent<NetIdentity>() && !gameObject.GetComponent<NetIdentity>().IsOwner())
        return;

    // Input only
    Input& input = ServiceLocator::Get<Input>();
    glm::vec2 movement{};

    // Horizontal movement
    if (input.IsKeyHeld(Key::A))
    {
        movement.x = -1.0f;
        m_Direction = Direction::Left;
    }
    else if (input.IsKeyHeld(Key::D))
    {
        movement.x = 1.0f;
        m_Direction = Direction::Right;
    }
    if (input.IsKeyHeld(Key::W))
    {
        movement.y = 1.0f;
        m_Direction = Direction::Up;
    }
    else if (input.IsKeyHeld(Key::S))
    {
        movement.y = -1.0f;
        m_Direction = Direction::Down;
    }

    if (m_IsGrounded && input.IsKeyPressed(Key::Space))
    {
        BClassRegistry::Get().Find<Boon::PlayerController>()->InvokeByName(this, "Jump");
    }

    // Normalize
    if (glm::length(movement) >= 1.0f)
        Move(glm::normalize(movement));
    else
        Move({});
}

void PlayerController::FixedUpdate(GameObject gameObject)
{
    if (!gameObject.HasComponent<Rigidbody2D>())
        return;

    TransformComponent& tc = gameObject.GetTransform();
    Rigidbody2D& rb = gameObject.GetComponent<Rigidbody2D>();

    const float dt = Time::Get().GetFixedTimeStep();
    glm::vec2 currentVel = rb.GetVelocity();

    if (m_MoveInput != glm::vec2(0.0f))
    {
        glm::vec2 targetVel = m_MoveInput * m_MovementSpeed;
        glm::vec2 force = targetVel * dt;

        rb.SetVelocity(targetVel);
    }
    else
    {
        rb.SetVelocity({});
    }
}

void PlayerController::OnBeginOverlap(GameObject gameObject, GameObject other)
{
    
}

void PlayerController::OnEndOverlap(GameObject gameObject, GameObject other)
{
    
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