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

		/**
		 * @brief Construct a perspective camera.
		 *	@param fov Field of view in degrees.
		 * @param near Near clipping plane distance.
		 * @param far Far clipping plane distance.
		 */
		Camera(float fov = 90.f, float near = 0.1f, float far = 10.0f);

		/**
		 * @brief Construct an orthographic camera.
		 *
		 * @param size Orthographic size.
		 * @param aspectRatio Aspect ratio (width/height).
		 * @param near Near clipping plane distance.
		 * @param far Far clipping plane distance.
		 */
		Camera(float size, float aspectRatio, float near, float far);

		/**
		 * @brief Destroy the Camera.
		 */
		~Camera();

		Camera(const Camera& other) = default;
		Camera(Camera&& other) = default;
		Camera& operator=(const Camera& other) = default;
		Camera& operator=(Camera&& other) = default;

		/**
		 * @brief Set the projection type (perspective or orthographic).
		 */
		void SetProjectionType(ProjectionType type);

		/**
		 * @brief Configure perspective projection parameters.
		 */
		void SetPerspective(float fov = 90.f, float near = 0.1f, float far = 10.0f);

		/**
		 * @brief Configure orthographic projection parameters.
		 */
		void SetOrthographic(float size, float aspectRatio, float near = 0.1, float far = 10.f);

		/**
		 * @brief Set the aspect ratio.
		 */
		void SetAspectRatio(float aspectRatio);

		/**
		 * @brief Set the aspect ratio using width and height.
		 */
		void SetAspectRatio(float width, float height);

		/**
		 * @brief Set orthographic size.
		 */
		void SetSize(float size);

		/**
		 * @brief Set field of view for perspective projection.
		 */
		void SetFov(float fov);

		/**
		 * @brief Set near clipping plane distance.
		 */
		void SetNear(float near);

		/**
		 * @brief Set far clipping plane distance.
		 */
		void SetFar(float far);

		/**
		 * @brief Get the current projection type.
		 */
		ProjectionType GetProjectionType() const;

		/**
		 * @brief Get the projection matrix.
		 *
		 * @return Const reference to the computed projection matrix.
		 */
		const glm::mat4& GetProjection();

		/**
		 * @brief Get the aspect ratio (width/height).
		 */
		float GetAspectRatio() const;

		/**
		 * @brief Get the orthographic size.
		 */
		float GetSize() const;

		/**
		 * @brief Get the field of view.
		 */
		float GetFov() const;

		/**
		 * @brief Get the near clipping plane distance.
		 */
		float GetNear() const;

		/**
		 * @brief Get the far clipping plane distance.
		 */
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