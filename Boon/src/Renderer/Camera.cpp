#include "Renderer/Camera.h"
#include "Core/BitFlag.h"
#include "glm/gtc/matrix_transform.hpp"

//temp
#include "Core/Application.h"

Boon::Camera::Camera(float fov, float width, float height, float near, float far)
	: m_Fov{ fov }, m_Size{ width, height }, m_Near{ near }, m_Far{ far }, m_Dirty{ true }, m_Type{ ProjectionType::Perspective }
{
	BitFlag::Set(m_Dirty, CameraFlags::PerspectiveDirty, true);

	Application::Get().GetOnWindowResize() += [this](int width, int height) {SetSize((float)width, (float)height); };
}

Boon::Camera::Camera(float width, float height, float near, float far)
	: m_Size{ width, height }, m_Near{ near }, m_Far{ far }, m_Dirty{ true }, m_Type{ ProjectionType::Orthographic }
{
	BitFlag::Set(m_Dirty, CameraFlags::PerspectiveDirty, true);

	Application::Get().GetOnWindowResize() += [this](int width, int height) {SetSize((float)width, (float)height); };
}

const glm::mat4& Boon::Camera::GetProjection()
{
	if (BitFlag::IsSet(m_Dirty, CameraFlags::PerspectiveDirty))
	{
		CalculateProjection(m_Projection, m_Near, m_Far);
		BitFlag::Set(m_Dirty, CameraFlags::PerspectiveDirty, false);
	}

	return m_Projection;
}

const glm::vec2& Boon::Camera::GetSize() const
{
	return m_Size;
}

float Boon::Camera::GetFov() const
{
	return m_Fov;
}

float Boon::Camera::GetNear() const
{
	return m_Near;
}

float Boon::Camera::GetFar() const
{
	return m_Far;
}

void Boon::Camera::CalculateProjection(glm::mat4& projection, float near, float far)
{
	switch (m_Type)
	{
	case ProjectionType::Perspective:
		projection = glm::perspectiveFov(glm::radians(m_Fov), m_Size.x, m_Size.y, near, far);
		break;

	case ProjectionType::Orthographic:
		projection = glm::ortho(-m_Size.x * 0.5f, m_Size.x * 0.5f, m_Size.y * 0.5f, -m_Size.y * 0.5f, near, far);
		break;
	}
}

void Boon::Camera::SetPerspective(float fov, float width, float height, float near, float far)
{
	m_Type = ProjectionType::Perspective;
	m_Fov = fov;
	m_Size = { width, height };
	m_Near = near;
	m_Far = far;
	BitFlag::Set(m_Dirty, CameraFlags::PerspectiveDirty, true);
}

void Boon::Camera::SetOrthographic(float width, float height, float near, float far)
{
	m_Type = ProjectionType::Orthographic;
	m_Size = { width, height };
	m_Near = near;
	m_Far = far;
	BitFlag::Set(m_Dirty, CameraFlags::PerspectiveDirty, true);
}

void Boon::Camera::SetWidth(float width)
{
	m_Size.x = width;
	BitFlag::Set(m_Dirty, CameraFlags::PerspectiveDirty, true);
}

void Boon::Camera::SetHeight(float height)
{
	m_Size.y = height;
	BitFlag::Set(m_Dirty, CameraFlags::PerspectiveDirty, true);
}

void Boon::Camera::SetSize(float width, float height)
{
	m_Size = { width, height };
	BitFlag::Set(m_Dirty, CameraFlags::PerspectiveDirty, true);
}

void Boon::Camera::SetFov(float fov)
{
	m_Fov = fov;
	BitFlag::Set(m_Dirty, CameraFlags::PerspectiveDirty, true);
}

void Boon::Camera::SetNear(float near)
{
	m_Near = near;
	BitFlag::Set(m_Dirty, CameraFlags::PerspectiveDirty, true);
}

void Boon::Camera::SetFar(float far)
{
	m_Far = far;
	BitFlag::Set(m_Dirty, CameraFlags::PerspectiveDirty, true);
}
