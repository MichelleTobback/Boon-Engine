#pragma once
#include "UBData.h"
#include "VertexData.h"

#include <memory>

namespace Boon
{
	class Scene;
	class Shader;
	class VertexInput;
	class VertexBuffer;
	class UniformBuffer;
	class Camera;
	class TransformComponent;
	class Framebuffer;
	class SceneRenderer final
	{ 
	public:
		SceneRenderer(Scene* pScene, bool isSwapchainTarget = false);
		SceneRenderer(Scene* pScene, int viewportWidth, int viewportHeight, bool isSwapchainTarget = false);
		virtual ~SceneRenderer();

		SceneRenderer(const SceneRenderer& other) = delete;
		SceneRenderer(SceneRenderer&& other) = delete;
		SceneRenderer& operator=(const SceneRenderer& other) = delete;
		SceneRenderer& operator=(SceneRenderer&& other) = delete;

		void Render(Camera* camera, TransformComponent* cameraTransform);

		Framebuffer* GetOutputTarget() const { return m_pOutputFB.get(); }

		void SetViewport(int width, int height);

	private:
		void StartBatch();
		void NextBatch();
		void Flush();

		void RenderQuad(const glm::mat4& transform, const glm::vec4& color, int gameObjectHandle);

		std::shared_ptr<VertexInput> m_pQuadVertexInput{};
		std::shared_ptr<VertexBuffer> m_pQuadVertexBuffer{};
		std::shared_ptr<Shader> m_pShader{};
		std::shared_ptr<UniformBuffer> m_pCameraUniformBuffer{};
		std::shared_ptr<Framebuffer> m_pOutputFB;
		UBData::Camera m_CameraData{};

		uint32_t m_QuadIndexCount{};
		QuadVertex* m_QuadVertexBufferBase{ nullptr };
		QuadVertex* m_QuadVertexBufferPtr{ nullptr };
		glm::vec4 m_QuadVertexPositions[4];

		Scene* m_pScene;

		int m_ViewportWidth, m_ViewportHeight;
		bool m_ViewportDirty{ false };
	};
}