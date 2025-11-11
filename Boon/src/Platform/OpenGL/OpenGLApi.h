#pragma once
#include "Renderer/RenderAPI.h"

namespace Boon
{
	class OpenGLApi final : public BaseRenderAPI
	{
	public:
		OpenGLApi() = default;
		virtual ~OpenGLApi() = default;

		OpenGLApi(const OpenGLApi& other) = delete;
		OpenGLApi(OpenGLApi&& other) = delete;
		OpenGLApi& operator=(const OpenGLApi& other) = delete;
		OpenGLApi& operator=(OpenGLApi&& other) = delete;

		virtual void Init() override;
		virtual void Shutdown() override;

		virtual void BeginFrame() override;
		virtual void EndFrame() override;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const std::shared_ptr<VertexInput>& vertexInput, uint32_t indexCount = 0) override;
		virtual void DrawLines(const std::shared_ptr<VertexInput>& vertexInput, uint32_t lineCount) override;
		virtual void DrawArrays(const std::shared_ptr<VertexInput>& vertexInput, uint32_t indexCount) override;
	};
}