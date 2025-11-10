#include "Component/Rigidbody2D.h"

#include "Core/Time.h"

#include <box2d/box2d.h>

using namespace Boon;

glm::vec2 Rigidbody2D::GetVelocity() const
{
    if (!IsValid()) 
        return { 0.0f, 0.0f };
    b2Vec2 v = b2Body_GetLinearVelocity(RuntimeBody);
    return { v.x, v.y };
}

void Rigidbody2D::SetVelocity(const glm::vec2& velocity)
{
    if (!IsValid()) 
        return;
    b2Body_SetLinearVelocity(RuntimeBody, { velocity.x, velocity.y });
}

float Rigidbody2D::GetAngularVelocity() const
{
    if (!IsValid()) 
        return 0.0f;
    return b2Body_GetAngularVelocity(RuntimeBody);
}

void Rigidbody2D::SetAngularVelocity(float angularVelocity)
{
    if (!IsValid()) 
        return;
    b2Body_SetAngularVelocity(RuntimeBody, angularVelocity);
}

void Rigidbody2D::SetPosition(const glm::vec2& position)
{
    if (!IsValid()) 
        return;

    b2Rot rot = b2Body_GetRotation(RuntimeBody);
    b2Body_SetTransform(RuntimeBody, { position.x, position.y }, rot);
}

void Rigidbody2D::SetRotation(float degrees)
{
    if (!IsValid()) 
        return;

    b2Vec2 pos = b2Body_GetPosition(RuntimeBody);
    b2Rot rot = b2MakeRot(glm::radians(degrees));
    b2Body_SetTransform(RuntimeBody, pos, rot);
}

void Rigidbody2D::SetTransform(const glm::vec2& position, float degrees)
{
    if (!IsValid()) 
        return;

    b2Rot rot = b2MakeRot(glm::radians(degrees));
    b2Body_SetTransform(RuntimeBody, { position.x, position.y }, rot);
}

void Rigidbody2D::MovePosition(const glm::vec2& targetPosition)
{
    if (!IsValid()) 
        return;

    // For kinematic bodies: teleport directly each frame
    if (Type == BodyType::Kinematic)
    {
        b2Rot rot = b2Body_GetRotation(RuntimeBody);
        b2Body_SetTransform(RuntimeBody, { targetPosition.x, targetPosition.y }, rot);
        return;
    }

    // For dynamic bodies: set velocity toward target
    b2Vec2 current = b2Body_GetPosition(RuntimeBody);
    b2Vec2 delta = { targetPosition.x - current.x, targetPosition.y - current.y };

    float invDt = 1.0f / Time::Get().GetFixedTimeStep();

    b2Body_SetLinearVelocity(RuntimeBody, { delta.x * invDt, delta.y * invDt });
}

void Rigidbody2D::MoveRotation(float targetDegrees)
{
    if (!IsValid()) 
        return;

    if (Type == BodyType::Kinematic)
    {
        b2Vec2 pos = b2Body_GetPosition(RuntimeBody);
        b2Rot rot = b2MakeRot(glm::radians(targetDegrees));
        b2Body_SetTransform(RuntimeBody, pos, rot);
        return;
    }

    float currentAngle = b2Rot_GetAngle(b2Body_GetRotation(RuntimeBody));
    float targetAngle = glm::radians(targetDegrees);
    float delta = targetAngle - currentAngle;

    float invDt = 1.0f / Time::Get().GetFixedTimeStep();

    b2Body_SetAngularVelocity(RuntimeBody, delta * invDt);
}

// --- Forces ---
void Rigidbody2D::AddForce(const glm::vec2& force, ForceMode mode, bool wake)
{
    if (!IsValid()) 
        return;

    if (mode == ForceMode::Force)
        b2Body_ApplyForceToCenter(RuntimeBody, { force.x, force.y }, wake);
    else
        b2Body_ApplyLinearImpulseToCenter(RuntimeBody, { force.x, force.y }, wake);
}

void Rigidbody2D::AddForceAtPosition(const glm::vec2& force, const glm::vec2& worldPos, ForceMode mode, bool wake)
{
    if (!IsValid()) 
        return;

    if (mode == ForceMode::Force)
        b2Body_ApplyForce(RuntimeBody, { force.x, force.y }, { worldPos.x, worldPos.y }, wake);
    else
        b2Body_ApplyLinearImpulse(RuntimeBody, { force.x, force.y }, { worldPos.x, worldPos.y }, wake);
}

// --- Torque ---
void Rigidbody2D::AddTorque(float torque, ForceMode mode, bool wake)
{
    if (!IsValid()) 
        return;

    if (mode == ForceMode::Force)
        b2Body_ApplyTorque(RuntimeBody, torque, wake);
    else
        b2Body_ApplyAngularImpulse(RuntimeBody, torque, wake);
}

// --- Impulses ---
void Rigidbody2D::AddImpulse(const glm::vec2& impulse, bool wake)
{
    if (!IsValid()) 
        return;
    b2Body_ApplyLinearImpulseToCenter(RuntimeBody, { impulse.x, impulse.y }, wake);
}

void Rigidbody2D::AddImpulseAtPosition(const glm::vec2& impulse, const glm::vec2& worldPos, bool wake)
{
    if (!IsValid()) 
        return;
    b2Body_ApplyLinearImpulse(RuntimeBody, { impulse.x, impulse.y }, { worldPos.x, worldPos.y }, wake);
}

// --- Mass & Inertia ---
float Rigidbody2D::GetMass() const
{
    if (!IsValid()) return 0.0f;
    return b2Body_GetMass(RuntimeBody);
}

void Rigidbody2D::SetMass(float mass)
{
    if (!IsValid()) return;

    b2MassData massData = b2Body_GetMassData(RuntimeBody);
    massData.mass = mass;
    b2Body_SetMassData(RuntimeBody, massData);
}

float Rigidbody2D::GetInertia() const
{
    if (!IsValid()) 
        return 0.0f;

    // Inertia is stored inside mass data in older versions
    b2MassData massData = b2Body_GetMassData(RuntimeBody);
    return massData.rotationalInertia;
}

void Rigidbody2D::RecalculateMassData()
{
    if (!IsValid()) 
        return;
    b2Body_ApplyMassFromShapes(RuntimeBody);
}

// --- Gravity & Damping ---
float Rigidbody2D::GetGravityScale() const
{
    if (!IsValid()) 
        return 1.0f;
    return b2Body_GetGravityScale(RuntimeBody);
}

void Rigidbody2D::SetGravityScale(float scale)
{
    if (!IsValid()) 
        return;
    b2Body_SetGravityScale(RuntimeBody, scale);
}

float Rigidbody2D::GetLinearDamping() const
{
    if (!IsValid()) 
        return 0.0f;
    return b2Body_GetLinearDamping(RuntimeBody);
}

void Rigidbody2D::SetLinearDamping(float damping)
{
    if (!IsValid()) 
        return;
    b2Body_SetLinearDamping(RuntimeBody, damping);
}

float Rigidbody2D::GetAngularDamping() const
{
    if (!IsValid()) 
        return 0.0f;
    return b2Body_GetAngularDamping(RuntimeBody);
}

void Rigidbody2D::SetAngularDamping(float damping)
{
    if (!IsValid()) 
        return;
    b2Body_SetAngularDamping(RuntimeBody, damping);
}

bool Rigidbody2D::IsAwake() const
{
    if (!IsValid()) 
        return false;
    return b2Body_IsAwake(RuntimeBody);
}

// --- Center of Mass ---
glm::vec2 Rigidbody2D::GetCenterOfMass() const
{
    if (!IsValid()) 
        return { 0.0f, 0.0f };
    b2MassData massData = b2Body_GetMassData(RuntimeBody);
    return { massData.center.x, massData.center.y };
}

glm::vec2 Rigidbody2D::GetWorldCenterOfMass() const
{
    if (!IsValid()) 
        return { 0.0f, 0.0f };

    b2Transform xf = b2Body_GetTransform(RuntimeBody);
    b2MassData massData = b2Body_GetMassData(RuntimeBody);

    // Transform local center to world space manually
    b2Vec2 world = b2TransformPoint(xf, massData.center);
    return { world.x, world.y };
}

void Rigidbody2D::SetCenterOfMass(const glm::vec2& localOffset)
{
    if (!IsValid()) 
        return;

    b2MassData massData = b2Body_GetMassData(RuntimeBody);
    massData.center = { localOffset.x, localOffset.y };
    b2Body_SetMassData(RuntimeBody, massData);
}

bool Rigidbody2D::IsValid() const { return b2Body_IsValid(RuntimeBody); }