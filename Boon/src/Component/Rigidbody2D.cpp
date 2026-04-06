#include "Component/Rigidbody2D.h"

using namespace Boon;

void Rigidbody2D::SetVelocity(const glm::vec2& velocity)
{
    PendingSetVelocity = velocity;
    HasPendingSetVelocity = true;
    DirtyVelocity = true;
    WakeRequested = true;
}

void Rigidbody2D::SetAngularVelocity(float angularVelocity)
{
    PendingSetAngularVelocity = angularVelocity;
    HasPendingSetAngularVelocity = true;
    DirtyVelocity = true;
    WakeRequested = true;
}

void Rigidbody2D::SetPosition(const glm::vec2& position)
{
    PendingSetPosition = position;
    PendingSetRotation = Rotation;
    HasPendingSetTransform = true;
    DirtyTransform = true;
    WakeRequested = true;
}

void Rigidbody2D::SetRotation(float degrees)
{
    PendingSetPosition = Position;
    PendingSetRotation = degrees;
    HasPendingSetTransform = true;
    DirtyTransform = true;
    WakeRequested = true;
}

void Rigidbody2D::SetTransform(const glm::vec2& position, float degrees)
{
    PendingSetPosition = position;
    PendingSetRotation = degrees;
    HasPendingSetTransform = true;
    DirtyTransform = true;
    WakeRequested = true;
}

void Rigidbody2D::AddForce(const glm::vec2& force, ForceMode mode, bool wake)
{
    if (mode == ForceMode::Force)
        PendingForce += force;
    else
        PendingImpulse += force;

    if (wake)
        WakeRequested = true;
}

void Rigidbody2D::AddTorque(float torque, ForceMode mode, bool wake)
{
    PendingTorque += torque;

    if (wake)
        WakeRequested = true;
}

void Rigidbody2D::AddImpulse(const glm::vec2& impulse, bool wake)
{
    PendingImpulse += impulse;

    if (wake)
        WakeRequested = true;
}