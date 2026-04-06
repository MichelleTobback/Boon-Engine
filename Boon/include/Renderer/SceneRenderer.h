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
		/**
		 * @brief Create a SceneRenderer for the provided scene.
		 *	@param pScene Pointer to the Scene to render.
		 * @param isSwapchainTarget If true, the renderer targets the swapchain (default false).
		 */
		SceneRenderer(Scene* pScene, bool isSwapchainTarget = false);

		/**
		 * @brief Create a SceneRenderer with an explicit viewport size.
		 */
		SceneRenderer(Scene* pScene, int viewportWidth, int viewportHeight, bool isSwapchainTarget = false);

		/**
		 * @brief Destroy the SceneRenderer and release rendering resources.
		 */
		virtual ~SceneRenderer();

		SceneRenderer(const SceneRenderer& other) = delete;
		SceneRenderer(SceneRenderer&& other) = delete;
		SceneRenderer& operator=(const SceneRenderer& other) = delete;
		SceneRenderer& operator=(SceneRenderer&& other) = delete;

		/**
		 * @brief Render the current scene using the optional camera.
		 *
		 * @param camera Optional camera to use for rendering. If nullptr a default may be used.
		 * @param cameraTransform Optional transform for the camera.
		 */
		void Render(Camera* camera = nullptr, TransformComponent* cameraTransform = nullptr);

		/**
		 * @brief Get the framebuffer used as the output target by the renderer.
		 *
		 * @return Pointer to the output Framebuffer, or nullptr if not set.
		 */
		Framebuffer* GetOutputTarget() const { return m_pOutputFB.get(); }

		/**
		 * @brief Set the viewport size used by the renderer.
		 */
		void SetViewport(int width, int height);

		/**
		 * @brief Set the Scene context for this renderer.
		 *
		 * If the context changes the renderer may mark internal state dirty.
		 */
		inline void SetContext(Scene* pScene) 
		{ 
			if (pScene != m_pScene)
				m_ViewportDirty = true;
			m_pScene = pScene; 
		}

		/**
		 * @brief Access the 2D renderer used internally by the scene renderer.
		 *	@return Pointer to the Renderer2D instance.
		 */
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