#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace Boon
{
	class VertexInput;
	class BaseRenderAPI;
	class Renderer final
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginFrame();
		static void EndFrame();

		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
		static void SetClearColor(const glm::vec4& color);
		static void Clear();
		
		static void DrawIndexed(const std::shared_ptr<VertexInput>& vertexInput, uint32_t indexCount = 0);
		static void DrawArrays(const std::shared_ptr<VertexInput>& vertexInput, uint32_t indexCount = 0);
		static void DrawLines(const std::shared_ptr<VertexInput>& vertexInput, uint32_t lineCount = 0);

	private:
		Renderer() = delete;
		static std::unique_ptr<BaseRenderAPI> s_pApi;
	};
}