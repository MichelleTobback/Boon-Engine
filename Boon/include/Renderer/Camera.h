#pragma once

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

		Camera(float fov = 90.f, float near = 0.1f, float far = 10.0f);
		Camera(float size, float aspectRatio, float near, float far);
		~Camera();

		Camera(const Camera& other) = default;
		Camera(Camera&& other) = default;
		Camera& operator=(const Camera& other) = default;
		Camera& operator=(Camera&& other) = default;

		void SetProjectionType(ProjectionType type);
		void SetPerspective(float fov = 90.f, float near = 0.1f, float far = 10.0f);
		void SetOrthographic(float size, float aspectRatio, float near = 0.1, float far = 10.f);
		void SetAspectRatio(float aspectRatio);
		void SetAspectRatio(float width, float height);
		void SetSize(float size);
		void SetFov(float fov);
		void SetNear(float near);
		void SetFar(float far);

		ProjectionType GetProjectionType() const;
		const glm::mat4& GetProjection();
		float GetAspectRatio() const;
		float GetSize() const;
		float GetFov() const;
		float GetNear() const;
		float GetFar() const;

	private:
		void CalculateProjection();

		glm::mat4 m_Projection{};
		ProjectionType m_Type;

		float m_OrthoSize;
		float m_AspectRatio;
		float m_Fov;
		float m_Near;
		float m_Far;

		enum class CameraFlags
		{
			None = 0,
			PerspectiveDirty = 1
		} m_Dirty{ };
	};
}