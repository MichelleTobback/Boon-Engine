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
		/**
		 * @brief Initialize the rendering subsystem.
		 */
		static void Init();

		/**
		 * @brief Shutdown the rendering subsystem and release resources.
		 */
		static void Shutdown();

		/**
		 * @brief Begin a new frame for rendering.
		 */
		static void BeginFrame();

		/**
		 * @brief End the current frame and submit commands.
		 */
		static void EndFrame();

		/**
		 * @brief Set the rendering viewport rectangle.
		 */
		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

		/**
		 * @brief Set the clear color used when clearing the framebuffer.
		 */
		static void SetClearColor(const glm::vec4& color);

		/**
		 * @brief Clear the current framebuffer using the configured clear color.
		 */
		static void Clear();
		
		/**
		 * @brief Draw geometry using indexed primitives.
		 *
		 * @param vertexInput Vertex input describing vertex/index buffers and layout.
		 * @param indexCount Number of indices to draw. If zero, the vertexInput may provide a default.
		 */
		static void DrawIndexed(const std::shared_ptr<VertexInput>& vertexInput, uint32_t indexCount = 0);

		/**
		 * @brief Draw arrays (non-indexed) geometry.
		 *
		 * @param vertexInput Vertex input describing buffers and layout.
		 * @param indexCount Number of vertices to draw.
		 */
		static void DrawArrays(const std::shared_ptr<VertexInput>& vertexInput, uint32_t indexCount = 0);

		/**
		 * @brief Draw line primitives.
		 *
		 * @param vertexInput Vertex input describing buffers and layout.
		 * @param lineCount Number of lines to draw.
		 */
		static void DrawLines(const std::shared_ptr<VertexInput>& vertexInput, uint32_t lineCount = 0);

	private:
		Renderer() = delete;
		static std::unique_ptr<BaseRenderAPI> s_pApi;
	};
}