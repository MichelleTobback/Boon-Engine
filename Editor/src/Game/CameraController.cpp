#include "game/CameraController.h"
#include <Core/Time.h>

#include <BoonDebug/DebugRenderer.h>

void Boon::CameraController::Awake(GameObject gameObject)
{
	
}

inline glm::vec3 SnapCameraToPixelGrid(const glm::vec3& pos, float ppu)
{
    return {
        floor(pos.x * ppu) / ppu,
        floor(pos.y * ppu) / ppu,
        pos.z
    };
}

void Boon::CameraController::Update(GameObject gameObject)
{
    if (!m_Target.IsValid())
        return;

    float dt = Time::Get().GetDeltaTime();
    if (dt <= 0.0f)
        return; // avoid divide-by-zero & weird spikes

    TransformComponent& camT = gameObject.GetTransform();

    glm::vec3 camPos = camT.GetLocalPosition();
    glm::vec3 targetPos = m_Target->GetWorldPosition();

    // ----- DEADZONE SETTINGS -----
    glm::vec2 deadzoneHalfSize = m_Deadzone * 0.5f;
    float softness = 0.4f;          // 0 = hard edges, 1 = very soft fade-out
    float smoothTime = m_FollowSpeed; // lower = snappier, higher = smoother
    // Clamp to avoid near-zero values causing crazy motion
    smoothTime = glm::max(0.0001f, smoothTime);
    // Optional: cap max follow speed to avoid huge jumps
    const float maxFollowSpeed = 100.0f;
    // -----------------------------

    glm::vec2 diff = glm::vec2(targetPos - camPos);

    // Soft movement based on how far outside the zone we are
    glm::vec2 move(0.0f);

    auto softZone = [&](float delta, float halfSize)
        {
            if (glm::abs(delta) <= halfSize)
                return 0.0f;

            float over = glm::abs(delta) - halfSize;

            // Avoid division by zero if softness is 0
            float soft = glm::max(softness, 0.0001f);

            // Smooth blend into motion the further you are out
            float t = glm::clamp(over / (halfSize * soft), 0.0f, 1.0f);
            t = t * t * (3.0f - 2.0f * t); // smoothstep

            return glm::sign(delta) * over * t;
        };

    move.x = softZone(diff.x, deadzoneHalfSize.x);
    move.y = softZone(diff.y, deadzoneHalfSize.y);

    glm::vec3 targetCamPos = camPos + glm::vec3(move, 0.0f);
    targetCamPos.z = camPos.z;

    // ---- ULTRA-SMOOTH MOTION (Unity-style SmoothDamp) ----
    auto smoothDamp = [&](float current, float target, float& velocity) -> float
        {
            // This is essentially Mathf.SmoothDamp
            float omega = 2.0f / smoothTime;
            float x = omega * dt;
            float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);

            float change = current - target;
            float originalTarget = target;

            // Clamp maximum speed so we don't snap too hard (prevents rubberband spikes)
            float maxChange = maxFollowSpeed * smoothTime;
            change = glm::clamp(change, -maxChange, maxChange);
            target = current - change;

            float temp = (velocity + omega * change) * dt;
            velocity = (velocity - omega * temp) * exp;

            float output = target + (change + temp) * exp;

            // Prevent overshooting the original target (key to killing rubberbanding)
            if ((originalTarget - current > 0.0f) == (output > originalTarget))
            {
                output = originalTarget;
                velocity = (output - originalTarget) / dt;
            }

            return output;
        };

    glm::vec3 newPos;
    newPos.x = smoothDamp(camPos.x, targetCamPos.x, m_Velocity.x);
    newPos.y = smoothDamp(camPos.y, targetCamPos.y, m_Velocity.y);
    newPos.z = camPos.z;

    camT.SetLocalPosition(newPos);

    // -------------------------------------------------------
    // DEBUG DRAW: deadzone rectangle (use newPos to avoid visual "lag")
    // -------------------------------------------------------
    DEBUG_DRAW_RECT(
        glm::vec3(newPos.x, newPos.y, 0.0f),
        m_Deadzone,
        glm::vec4(1.0f, 0.9f, 0.2f, 1.0f)
    );

    // Optional: draw target
    //DEBUG_DRAW_RECT(targetPos, glm::vec2(0.25f), glm::vec4(1.f, 0.f, 0.f, 1.f));
}

