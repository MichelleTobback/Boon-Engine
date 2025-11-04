#include "Renderer/Camera.h"

#include "Core/BitFlag.h"

#include <glm/gtc/matrix_transform.hpp>

Boon::Camera::Camera(float fov, float near, float far)
	: m_Fov{ fov }, m_OrthoSize{ 1.f }, m_Near{ near }, m_Far{ far }, m_Dirty{ true }, m_Type{ ProjectionType::Perspective }
{
	BitFlag::Set(m_Dirty, CameraFlags::PerspectiveDirty, true);
}

Boon::Camera::Camera(float size, float aspectRatio, float near, float far)
	: m_OrthoSize{ size }, m_Near{ near }, m_Far{ far }, m_Dirty{ true }, m_Type{ ProjectionType::Orthographic }
{
	BitFlag::Set(m_Dirty, CameraFlags::PerspectiveDirty, true);
}

Boon::Camera::~Camera()
{
	
}

Boon::Camera::ProjectionType Boon::Camera::GetProjectionType() const
{
	return m_Type;
}

const glm::mat4& Boon::Camera::GetProjection()
{
	if (BitFlag::IsSet(m_Dirty, CameraFlags::PerspectiveDirty))
	{
		CalculateProjection();
		BitFlag::Set(m_Dirty, CameraFlags::PerspectiveDirty, false);
	}

	return m_Projection;
}

float Boon::Camera::GetSize() const
{
	return m_OrthoSize;
}

float Boon::Camera::GetAspectRatio() const
{
	return m_AspectRatio;
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

void Boon::Camera::CalculateProjection()
{
	switch (m_Type)
	{
	case ProjectionType::Perspective:
		m_Projection = glm::perspective(glm::radians(m_Fov), m_AspectRatio, m_Near, m_Far);
		break;

	case ProjectionType::Orthographic:
		float orthoLeft = -m_OrthoSize * m_AspectRatio * 0.5f;
		float orthoRight = m_OrthoSize * m_AspectRatio * 0.5f;
		float orthoBottom = -m_OrthoSize * 0.5f;
		float orthoTop = m_OrthoSize * 0.5f;

		m_Projection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, m_Near, m_Far);
		break;
	}
}

void Boon::Camera::SetProjectionType(ProjectionType type)
{
	m_Type = type;
	BitFlag::Set(m_Dirty, CameraFlags::PerspectiveDirty, true);
}

void Boon::Camera::SetPerspective(float fov, float near, float far)
{
	m_Type = ProjectionType::Perspective;
	m_Fov = fov;
	m_Near = near;
	m_Far = far;
	BitFlag::Set(m_Dirty, CameraFlags::PerspectiveDirty, true);
}

void Boon::Camera::SetOrthographic(float size, float aspectRatio, float near, float far)
{
	m_Type = ProjectionType::Orthographic;
	m_OrthoSize = size;
	m_AspectRatio = aspectRatio;
	m_Near = near;
	m_Far = far;
	BitFlag::Set(m_Dirty, CameraFlags::PerspectiveDirty, true);
}

void Boon::Camera::SetAspectRatio(float aspectRatio)
{
	m_AspectRatio = aspectRatio;
	BitFlag::Set(m_Dirty, CameraFlags::PerspectiveDirty, true);
}

void Boon::Camera::SetAspectRatio(float width, float height)
{
	m_AspectRatio = width / height;
	BitFlag::Set(m_Dirty, CameraFlags::PerspectiveDirty, true);
}

void Boon::Camera::SetSize(float size)
{
	m_OrthoSize = size;
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
