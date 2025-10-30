#pragma once
#include <memory>
#include "UBData.h"

namespace Boon
{
	class Shader;
	class VertexInput;
	class UniformBuffer;
	class Camera;
	class TransformComponent;
	class SceneRenderer final
	{ 
	public:
		SceneRenderer();
		virtual ~SceneRenderer() = default;

		SceneRenderer(const SceneRenderer& other) = delete;
		SceneRenderer(SceneRenderer&& other) = delete;
		SceneRenderer& operator=(const SceneRenderer& other) = delete;
		SceneRenderer& operator=(SceneRenderer&& other) = delete;

		void Render(Camera* camera, TransformComponent* cameraTransform);

	private:
		std::shared_ptr<VertexInput> m_pQuadVertexInput{};
		std::shared_ptr<Shader> m_pShader{};
		std::shared_ptr<UniformBuffer> m_pCameraUniformBuffer{};
		UBData::Camera m_CameraData{};

		//temp
		std::shared_ptr<Camera> m_pCamera;
	};
}