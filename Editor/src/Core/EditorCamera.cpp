#include "Core/EditorCamera.h"
#include "Core/ServiceLocator.h"
#include "Core/Time.h"
#include "Input/Input.h"

using namespace BoonEditor;
using namespace Boon;

BoonEditor::EditorCamera::EditorCamera(float width, float height)
    : EditorCamera(2.f, width / height, 0.1f, 1.f){}

BoonEditor::EditorCamera::EditorCamera(float size, float aspectRatio, float near, float far)
    : m_OrthoCamera{ size, aspectRatio, near, far }, m_PerspCamera{}
{
    m_OrthoTransform.SetLocalPosition(0.f, 0.f, 1.f);
    m_PerspTransform.SetLocalPosition(0.f, 0.f, 1.f);
}

BoonEditor::EditorCamera::~EditorCamera(){}

void BoonEditor::EditorCamera::Update()
{
    if (!m_Active)
        return;

    switch (m_Mode)
    {
    case Boon::Camera::ProjectionType::Orthographic:
        UpdateOrthographicController();
        break;
    case Boon::Camera::ProjectionType::Perspective:
        UpdatePerspectiveController();
        break;
    }
}

void BoonEditor::EditorCamera::Resize(float width, float height)
{
    float aspect = width / height;
    m_OrthoCamera.SetAspectRatio(aspect);
    m_PerspCamera.SetAspectRatio(aspect);
}

void BoonEditor::EditorCamera::UpdatePerspectiveController()
{
    Input& input{ ServiceLocator::Get<Input>() };

    static glm::vec2 prevMousePos = { input.GetMouseX(), input.GetMouseY() };
    glm::vec2 mousePos = { input.GetMouseX(), input.GetMouseY() };

    if (m_Active)
    {
        glm::vec3 forwardVector = m_PerspTransform.GetForward();
        glm::vec3 rightVector = m_PerspTransform.GetRight();
        glm::vec3 upVector = m_PerspTransform.GetUp();
        const float moveSpeed = input.IsKeyHeld(Key::LeftShift) ? 5.f : 1.6f;
        const float rotateSpeed = 15.0f;
        const float dt = Time::Get().GetDeltaTime();

        // Movement
        if (input.IsKeyHeld(Key::W))
            m_PerspTransform.Translate(dt * moveSpeed * forwardVector);
        if (input.IsKeyHeld(Key::S))
            m_PerspTransform.Translate(-dt * moveSpeed * forwardVector);
        if (input.IsKeyHeld(Key::A))
            m_PerspTransform.Translate(-dt * moveSpeed * rightVector);
        if (input.IsKeyHeld(Key::D))
            m_PerspTransform.Translate(dt * moveSpeed * rightVector);

        // Mouse rotation handling
        glm::vec2 deltaMouse = mousePos - prevMousePos;
        if (input.IsMouseHeld(Mouse::ButtonRight) && input.IsMouseHeld(Mouse::ButtonLeft))
        {
            m_PerspTransform.Translate({ dt * deltaMouse.x, dt * deltaMouse.y, 0.f }); // Elevate the camera
        }
        else if (input.IsMouseHeld(Mouse::ButtonRight))
        {
            // Apply rotation
            float yaw = rotateSpeed * dt * -deltaMouse.x;
            float pitch = rotateSpeed * dt * -deltaMouse.y;
            m_PerspTransform.Rotate(pitch, yaw, 0.0f);
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
        glm::vec3 forwardVector = m_OrthoTransform.GetForward();
        glm::vec3 rightVector = m_OrthoTransform.GetRight();
        glm::vec3 upVector = m_OrthoTransform.GetUp();
        const float moveSpeed = input.IsKeyHeld(Key::LeftShift) ? 0.9f : 0.4f;
        const float rotateSpeed = 15.0f;
        const float dt = Time::Get().GetDeltaTime();

        float size = m_OrthoCamera.GetSize();

        // Movement
        if (input.IsKeyHeld(Key::W))
            m_OrthoTransform.Translate(dt * moveSpeed * size * upVector);
        if (input.IsKeyHeld(Key::S))
            m_OrthoTransform.Translate(-dt * moveSpeed * size * upVector);
        if (input.IsKeyHeld(Key::A))
            m_OrthoTransform.Translate(-dt * moveSpeed * size * rightVector);
        if (input.IsKeyHeld(Key::D))
            m_OrthoTransform.Translate(dt * moveSpeed * size * rightVector);

        // Mouse rotation handling
        glm::vec2 deltaMouse = mousePos - prevMousePos;
        if (input.IsMouseHeld(Mouse::ButtonMiddle))
        {
            m_OrthoTransform.Translate({ dt * -deltaMouse.x, dt * deltaMouse.y, 0.f }); // Elevate the camera
        }
        else if (input.IsMouseHeld(Mouse::ButtonRight))
        {
            float zoomFactor = 0.24f; // feel free to tweak

            // Exponential zoom: scales with current zoom level
            size += deltaMouse.y * dt * zoomFactor * size;

            // clamp so we never hit zero or negative zoom
            size = glm::clamp(size, 0.05f, 500.0f);

            m_OrthoCamera.SetSize(size);
        }
    }

    prevMousePos = mousePos;
}

Camera& BoonEditor::EditorCamera::GetCamera()
{
    return m_Mode == Camera::ProjectionType::Orthographic ? m_OrthoCamera : m_PerspCamera;
}

const Camera& BoonEditor::EditorCamera::GetCamera() const
{
    return m_Mode == Camera::ProjectionType::Orthographic ? m_OrthoCamera : m_PerspCamera;
}

const Camera& BoonEditor::EditorCamera::GetOrthographicCamera() const
{
    return m_OrthoCamera;
}

Camera& BoonEditor::EditorCamera::GetOrthographicCamera()
{
    return m_OrthoCamera;
}

const Camera& BoonEditor::EditorCamera::GetPerspectiveCamera() const
{
    return m_PerspCamera;
}

Camera& BoonEditor::EditorCamera::GetPerspectiveCamera()
{
    return m_PerspCamera;
}

TransformComponent& BoonEditor::EditorCamera::GetTransform()
{
    return m_Mode == Camera::ProjectionType::Orthographic ? m_OrthoTransform : m_PerspTransform;
}

const TransformComponent& BoonEditor::EditorCamera::GetTransform() const
{
    return m_Mode == Camera::ProjectionType::Orthographic ? m_OrthoTransform : m_PerspTransform;
}

const TransformComponent& BoonEditor::EditorCamera::GetOrthographicTransform() const
{
    return m_OrthoTransform;
}

TransformComponent& BoonEditor::EditorCamera::GetOrthographicTransform()
{
    return m_OrthoTransform;
}

const TransformComponent& BoonEditor::EditorCamera::GetPerspectiveTransform() const
{
    return m_PerspTransform;
}

TransformComponent& BoonEditor::EditorCamera::GetPerspectiveTransform()
{
    return m_PerspTransform;
}

glm::mat4 BoonEditor::EditorCamera::GetView()
{
    return glm::inverse(GetTransform().GetWorld());
}

void BoonEditor::EditorCamera::SetActive(bool active)
{
    m_Active = active;
}
