#pragma once
#include "UBData.h"

#include <memory>
#include <array>

namespace Boon
{
	class Material;
	class Shader;
	class Renderer2D;
	class Renderer3D;
	class Scene;
	class UniformBuffer;
	class Camera;
	class TransformComponent;
	class EventBus;
	class AssetLibrary;
	class Framebuffer;
	class RenderPass;
	struct RenderContext;

	struct SceneRendererCreateInfo
	{
		AssetLibrary* AssetLib = nullptr;
		EventBus* Events = nullptr;
		Scene* pScene = nullptr;
		uint32_t Width = 1080;
		uint32_t Height = 720;
		bool bIsSwapchainTarget = false;
	};

	class SceneRenderer final
	{
	public:
		/**
		 * @brief Create a SceneRenderer for the provided scene.
		 *	@param pScene Pointer to the Scene to render.
		 * @param isSwapchainTarget If true, the renderer targets the swapchain (default false).
		 */
		SceneRenderer(const SceneRendererCreateInfo& desc);

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

		std::shared_ptr<Material> GetDefaultQuadMaterial() const { return m_pDefaultQuadMaterial; }
		std::shared_ptr<Material> GetDefaultTilemapMaterial() const { return m_pDefaultTilemapMaterial; }

		void AddPass(std::unique_ptr<RenderPass> pass);
		void SortPasses();

	private:
		void BeginScene(RenderContext& ctx, Camera* camera = nullptr, TransformComponent* cameraTransform = nullptr);
		void EndScene(RenderContext& ctx);

		std::unique_ptr<Renderer2D> m_pRenderer2D;
		std::unique_ptr<Renderer3D> m_pRenderer3D;
		std::shared_ptr<UniformBuffer> m_pCameraUniformBuffer{};
		std::shared_ptr<UniformBuffer> m_pObjectUniformBuffer{};
		std::shared_ptr<Framebuffer> m_pOutputFB;
		UBData::Camera m_CameraData{};
		UBData::Object m_ObjectData{};

		std::vector<std::unique_ptr<RenderPass>> m_Passes;
		bool m_bPassesDirty = false;

		std::shared_ptr<Material> m_pDefaultQuadMaterial;
		std::shared_ptr<Material> m_pDefaultTilemapMaterial;

		Scene* m_pScene;

		int m_ViewportWidth, m_ViewportHeight;
		bool m_ViewportDirty{ false };
	};
}