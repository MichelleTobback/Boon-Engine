#pragma once
#include "DebugRenderer/EditorViewportSettings.h"

#include <memory>

namespace Boon
{
	class Framebuffer;
	class Scene;
	class Renderer2D;
	class Camera;
	class TransformComponent;
}

using namespace Boon;

namespace BoonEditor
{
	class DebugRenderer final
	{
	public:
		DebugRenderer(Scene* pScene, Framebuffer* pFramebuffer);
		~DebugRenderer();

		void Render(EditorViewportSettings settings, Camera* camera = nullptr, TransformComponent* cameraTransform = nullptr);

		inline void SetContext(Scene* pScene) { m_pScene = pScene; }

	private:
		void Begin();
		void End();

		Scene* m_pScene;
		Framebuffer* m_pFramebuffer;
		std::unique_ptr<Renderer2D> m_pRenderer2D;
	};
}