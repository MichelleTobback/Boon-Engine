#pragma once
#include "EditorObject.h"
#include "Renderer/Camera.h"
#include "Component/TransformComponent.h"

using namespace Boon;

namespace BoonEditor
{
	class EditorCamera final : public EditorObject
	{
	public:
		EditorCamera(float width, float height);
		EditorCamera(float size, float aspectRatio, float near, float far);
		virtual ~EditorCamera();

		EditorCamera(const EditorCamera& other) = default;
		EditorCamera(EditorCamera&& other) = default;
		EditorCamera& operator=(const EditorCamera& other) = default;
		EditorCamera& operator=(EditorCamera&& other) = default;

		virtual void Update() override;

		void Resize(float width, float height);

		const TransformComponent& GetTransform() const;
		TransformComponent& GetTransform();
		const TransformComponent& GetOrthographicTransform() const;
		TransformComponent& GetOrthographicTransform();
		const TransformComponent& GetPerspectiveTransform() const;
		TransformComponent& GetPerspectiveTransform();

		const Camera& GetCamera() const;
		Camera& GetCamera();
		const Camera& GetOrthographicCamera() const;
		Camera& GetOrthographicCamera();
		const Camera& GetPerspectiveCamera() const;
		Camera& GetPerspectiveCamera();

		glm::mat4 GetView();

		inline void SetMode(Camera::ProjectionType mode) { m_Mode = mode; }
		inline Camera::ProjectionType GetMode() const { return m_Mode; }

		void SetActive(bool active);
		inline bool GetActive() const { return m_Active; }

	private:
		void UpdatePerspectiveController();
		void UpdateOrthographicController();

		Camera m_OrthoCamera, m_PerspCamera;
		TransformComponent m_OrthoTransform{nullptr}, m_PerspTransform{nullptr};
		bool m_Active{ false };
		Camera::ProjectionType m_Mode{ Camera::ProjectionType::Orthographic };
	};
}