#pragma once
#include "UBData.h"

#include <memory>
#include <array>

namespace Boon
{
	class Shader;
	class Renderer2D;
	class Scene;
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

		void Render(Camera* camera = nullptr, TransformComponent* cameraTransform = nullptr);

		Framebuffer* GetOutputTarget() const { return m_pOutputFB.get(); }

		void SetViewport(int width, int height);

		inline void SetContext(Scene* pScene) { m_pScene = pScene; m_ViewportDirty = true; }

		Renderer2D* GetRenderer2D() const;

	private:
		void BeginScene(Camera* camera = nullptr, TransformComponent* cameraTransform = nullptr);
		void EndScene();

		std::unique_ptr<Renderer2D> m_pRenderer2D;
		std::shared_ptr<UniformBuffer> m_pCameraUniformBuffer{};
		std::shared_ptr<UniformBuffer> m_pObjectUniformBuffer{};
		std::shared_ptr<Framebuffer> m_pOutputFB;
		std::shared_ptr<Shader> m_pTilemapShader;
		UBData::Camera m_CameraData{};
		UBData::Object m_ObjectData{};

		Scene* m_pScene;

		int m_ViewportWidth, m_ViewportHeight;
		bool m_ViewportDirty{ false };
	};
}