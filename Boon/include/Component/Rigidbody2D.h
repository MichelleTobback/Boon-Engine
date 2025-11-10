#pragma once
#include <box2d/id.h>
#include <glm/glm.hpp>

namespace Boon
{
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
            Force,    // Continuous (applied over time)
            Impulse   // Instantaneous (change in velocity)
        };

        BodyType Type = BodyType::Static;
        bool FixedRotation = false;

        b2BodyId RuntimeBody = b2_nullBodyId;

        // --- Velocity ---
        glm::vec2 GetVelocity() const;
        void SetVelocity(const glm::vec2& velocity);

        float GetAngularVelocity() const;
        void SetAngularVelocity(float angularVelocity);

        // --- Position / Rotation ---
        void SetPosition(const glm::vec2& position);
        void SetRotation(float degrees);
        void SetTransform(const glm::vec2& position, float degrees);
        void MovePosition(const glm::vec2& targetPosition);
        void MoveRotation(float targetDegrees);

        // --- Forces ---
        void AddForce(const glm::vec2& force, ForceMode mode = ForceMode::Force, bool wake = true);
        void AddForceAtPosition(const glm::vec2& force, const glm::vec2& worldPos, ForceMode mode = ForceMode::Force, bool wake = true);

        // --- Torque ---
        void AddTorque(float torque, ForceMode mode = ForceMode::Force, bool wake = true);

        // --- Impulses ---
        void AddImpulse(const glm::vec2& impulse, bool wake = true);
        void AddImpulseAtPosition(const glm::vec2& impulse, const glm::vec2& worldPos, bool wake = true);

        // --- Mass & Inertia ---
        float GetMass() const;
        void SetMass(float mass);
        float GetInertia() const;
        void RecalculateMassData();

        // --- Gravity & Damping ---
        float GetGravityScale() const;
        void SetGravityScale(float scale);

        float GetLinearDamping() const;
        void SetLinearDamping(float damping);

        float GetAngularDamping() const;
        void SetAngularDamping(float damping);

        // --- Center of Mass ---
        glm::vec2 GetCenterOfMass() const;
        glm::vec2 GetWorldCenterOfMass() const;
        void SetCenterOfMass(const glm::vec2& localOffset);

        bool IsAwake() const;

    private:
        friend class PhysicsWorld2D;
        bool IsValid() const;

        glm::vec2 PrevPosition;
        float PrevRotation;
	};
}