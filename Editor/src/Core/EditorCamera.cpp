#include "Core/EditorCamera.h"
#include "Core/ServiceLocator.h"
#include "Core/Time.h"
#include "Input/Input.h"

using namespace BoonEditor;
using namespace Boon;

BoonEditor::EditorCamera::EditorCamera(float width, float height)
    : m_Camera{ width, height }
{
    m_Transform.SetLocalPosition(0.f, 0.f, 1.f);
    m_Camera.SetOrthographic(width, height);
}

BoonEditor::EditorCamera::EditorCamera(float fov, float width, float height, float near, float far)
    : m_Camera{ fov, width, height, near, far }
{
    m_Transform.SetLocalPosition(0.f, 0.f, 1.f);
}

BoonEditor::EditorCamera::~EditorCamera(){}

void BoonEditor::EditorCamera::Update()
{
    switch (m_Camera.GetProjectionType())
    {
    case Boon::Camera::ProjectionType::Orthographic:
        UpdateOrthographicController();
        break;
    case Boon::Camera::ProjectionType::Perspective:
        UpdatePerspectiveController();
        break;
    }
}

void BoonEditor::EditorCamera::UpdatePerspectiveController()
{
    Input& input{ ServiceLocator::Get<Input>() };

    static glm::vec2 prevMousePos = { input.GetMouseX(), input.GetMouseY() };
    glm::vec2 mousePos = { input.GetMouseX(), input.GetMouseY() };

    if (m_Active)
    {
        glm::vec3 forwardVector = m_Transform.GetForward();
        glm::vec3 rightVector = m_Transform.GetRight();
        glm::vec3 upVector = m_Transform.GetUp();
        const float moveSpeed = input.IsKeyHeld(Key::LeftShift) ? 5.f : 1.6f;
        const float rotateSpeed = 15.0f;
        const float dt = Time::Get().GetDeltaTime();

        // Movement
        if (input.IsKeyHeld(Key::W))
            m_Transform.Translate(dt * moveSpeed * forwardVector);
        if (input.IsKeyHeld(Key::S))
            m_Transform.Translate(-dt * moveSpeed * forwardVector);
        if (input.IsKeyHeld(Key::A))
            m_Transform.Translate(-dt * moveSpeed * rightVector);
        if (input.IsKeyHeld(Key::D))
            m_Transform.Translate(dt * moveSpeed * rightVector);

        // Mouse rotation handling
        glm::vec2 deltaMouse = mousePos - prevMousePos;
        if (input.IsMouseHeld(Mouse::ButtonRight) && input.IsMouseHeld(Mouse::ButtonLeft))
        {
            m_Transform.Translate({ dt * deltaMouse.x, dt * deltaMouse.y, 0.f }); // Elevate the camera
        }
        else if (input.IsMouseHeld(Mouse::ButtonRight))
        {
            // Apply rotation
            float yaw = rotateSpeed * dt * -deltaMouse.x;
            float pitch = rotateSpeed * dt * -deltaMouse.y;
            m_Transform.Rotate(pitch, yaw, 0.0f);
        }
    }

    prevMousePos = mousePos;
}

void BoonEditor::EditorCamera::UpdateOrthographicController()
{
    Input& input{ ServiceLocator::Get<Input>() };

    static glm::vec2 prevMousePos = { input.GetMouseX(), input.GetMouseY() };
    glm::vec2 mousePos = { input.GetMouseX(), input.GetMouseY() };

    if (m_Active)
    {
        glm::vec3 forwardVector = m_Transform.GetForward();
        glm::vec3 rightVector = m_Transform.GetRight();
        glm::vec3 upVector = m_Transform.GetUp();
        const float moveSpeed = input.IsKeyHeld(Key::LeftShift) ? 5.f : 1.6f;
        const float rotateSpeed = 15.0f;
        const float dt = Time::Get().GetDeltaTime();

        // Movement
        if (input.IsKeyHeld(Key::W))
            m_Transform.Translate(-dt * moveSpeed * upVector);
        if (input.IsKeyHeld(Key::S))
            m_Transform.Translate(dt * moveSpeed * upVector);
        if (input.IsKeyHeld(Key::A))
            m_Transform.Translate(-dt * moveSpeed * rightVector);
        if (input.IsKeyHeld(Key::D))
            m_Transform.Translate(dt * moveSpeed * rightVector);

        // Mouse rotation handling
        glm::vec2 deltaMouse = mousePos - prevMousePos;
        if (input.IsMouseHeld(Mouse::ButtonRight))
        {
            glm::vec2 size{ m_Camera.GetSize() };
            float aspect{ size.x / size.y };
            m_Camera.SetSize( size.x + dt * deltaMouse.y * aspect, size.y + dt * deltaMouse.y );
        }
    }

    prevMousePos = mousePos;
}

Camera& BoonEditor::EditorCamera::GetCamera()
{
    return m_Camera;
}

const Camera& BoonEditor::EditorCamera::GetCamera() const
{
    return m_Camera;
}

TransformComponent& BoonEditor::EditorCamera::GetTransform()
{
    return m_Transform;
}

const TransformComponent& BoonEditor::EditorCamera::GetTransform() const
{
    return m_Transform;
}

glm::mat4 BoonEditor::EditorCamera::GetView()
{
    return glm::inverse(m_Transform.GetWorld());
}

void BoonEditor::EditorCamera::SetActive(bool active)
{
    m_Active = active;
}
