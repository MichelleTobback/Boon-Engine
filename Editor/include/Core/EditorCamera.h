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
		EditorCamera(float fov, float width, float height, float near = 0.1f, float far = 500.0f);
		virtual ~EditorCamera();

		EditorCamera(const EditorCamera& other) = default;
		EditorCamera(EditorCamera&& other) = default;
		EditorCamera& operator=(const EditorCamera& other) = default;
		EditorCamera& operator=(EditorCamera&& other) = default;

		virtual void Update() override;

		const TransformComponent& GetTransform() const;
		TransformComponent& GetTransform();
		const Camera& GetCamera() const;
		Camera& GetCamera();
		glm::mat4 GetView();

		void SetActive(bool active);

	private:
		void UpdatePerspectiveController();
		void UpdateOrthographicController();

		Camera m_Camera;
		TransformComponent m_Transform{ nullptr };
		bool m_Active{ false };
	};
}