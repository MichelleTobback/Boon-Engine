#pragma once
#include <memory>

namespace Boon
{
	class Shader;
	class VertexInput;
	class SceneRenderer final
	{ 
	public:
		SceneRenderer();
		virtual ~SceneRenderer() = default;

		SceneRenderer(const SceneRenderer& other) = delete;
		SceneRenderer(SceneRenderer&& other) = delete;
		SceneRenderer& operator=(const SceneRenderer& other) = delete;
		SceneRenderer& operator=(SceneRenderer&& other) = delete;

		void Render();

	private:
		std::shared_ptr<VertexInput> m_pQuadVertexInput{};
		std::shared_ptr<Shader> m_pShader{};
	};
}