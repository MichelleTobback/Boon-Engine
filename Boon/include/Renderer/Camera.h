#pragma once
#include "Event/Event.h"

#include <glm/glm.hpp>

namespace Boon
{
	class Camera final
	{
	public:
		enum class ProjectionType
		{
			Perspective = 0, Orthographic = 1
		};

		Camera() = default;
		Camera(float fov, float width, float height, float near = 0.1f, float far = 10.0f);
		Camera(float width, float height, float near = 0.1, float far = 1.f);
		~Camera();

		Camera(const Camera& other) = default;
		Camera(Camera&& other) = default;
		Camera& operator=(const Camera& other) = default;
		Camera& operator=(Camera&& other) = default;

		void SetPerspective(float fov, float width, float height, float near = 0.1f, float far = 10.0f);
		void SetOrthographic(float width, float height, float near = 0.1, float far = 1.f);
		void SetWidth(float width);
		void SetHeight(float height);
		void SetSize(float width, float height);
		void SetFov(float fov);
		void SetNear(float near);
		void SetFar(float far);

		const glm::mat4& GetProjection();
		const glm::vec2& GetSize() const;
		float GetFov() const;
		float GetNear() const;
		float GetFar() const;

	private:
		void CalculateProjection(glm::mat4& projection, float near, float far);

		glm::mat4 m_Projection{};
		glm::vec2 m_Size;
		ProjectionType m_Type;
		float m_Fov;
		float m_Near;
		float m_Far;

		EventListenerID m_WindowResizeEvent;

		enum class CameraFlags
		{
			None = 0,
			PerspectiveDirty = 1
		} m_Dirty{ };
	};
}