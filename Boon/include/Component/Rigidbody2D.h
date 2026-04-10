#pragma once
#include "Core/Boon.h"
#include <glm/glm.hpp>

namespace Boon
{
    BCLASS()
    struct Rigidbody2D final
    {
        enum class BodyType
        {
            Static = 0,
            Dynamic = 1,
            Kinematic = 2
        };

        enum class ForceMode
        {
            Force,
            Impulse
        };

        BPROPERTY()
        int Type = (int)BodyType::Static;

        BPROPERTY()
        float GravityScale = 1.f;

        BPROPERTY()
        bool FixedRotation = false;

        BPROPERTY()
        float LinearDamping = 0.0f;

        BPROPERTY()
        float AngularDamping = 0.0f;

        BPROPERTY()
        float Mass = 1.0f;

        // Cached state written by host physics
        glm::vec2 Velocity{ 0.0f, 0.0f };
        float AngularVelocity = 0.0f;

        glm::vec2 Position{ 0.0f, 0.0f };
        float Rotation = 0.0f;

        bool Awake = true;
        bool OnGround = false;

        // Requests
        glm::vec2 PendingForce{ 0.0f, 0.0f };
        glm::vec2 PendingImpulse{ 0.0f, 0.0f };
        float PendingTorque = 0.0f;

        bool WakeRequested = false;

        bool HasPendingSetVelocity = false;
        glm::vec2 PendingSetVelocity{ 0.0f, 0.0f };

        bool HasPendingSetAngularVelocity = false;
        float PendingSetAngularVelocity = 0.0f;

        bool HasPendingSetTransform = false;
        glm::vec2 PendingSetPosition{ 0.0f, 0.0f };
        float PendingSetRotation = 0.0f;

        bool DirtyBodyDef = true;
        bool DirtyVelocity = false;
        bool DirtyTransform = false;
        bool DirtyDamping = true;

        glm::vec2 GetVelocity() const { return Velocity; }
        void SetVelocity(const glm::vec2& velocity);

        float GetAngularVelocity() const { return AngularVelocity; }
        void SetAngularVelocity(float angularVelocity);

        float GetMass() const { return Mass; }
        void SetMass(float mass) { Mass = mass; }

        void SetPosition(const glm::vec2& position);
        void SetRotation(float degrees);
        void SetTransform(const glm::vec2& position, float degrees);

        void AddForce(const glm::vec2& force, ForceMode mode = ForceMode::Force, bool wake = true);
        void AddTorque(float torque, bool wake = true);
        void AddImpulse(const glm::vec2& impulse, bool wake = true);

        bool IsAwake() const { return Awake; }
    };
}