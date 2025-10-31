#pragma once
#include <memory>
#include <glm/glm.hpp>

namespace Boon
{
	enum class ERenderAPI
	{
		None,
		OpenGL
	};

	class VertexInput;
	class BaseRenderAPI
	{
	public:
		BaseRenderAPI() = default;
		virtual ~BaseRenderAPI() = default;

		BaseRenderAPI(const BaseRenderAPI& other) = delete;
		BaseRenderAPI(BaseRenderAPI&& other) = delete;
		BaseRenderAPI& operator=(const BaseRenderAPI& other) = delete;
		BaseRenderAPI& operator=(BaseRenderAPI&& other) = delete;

		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;
		
		virtual void DrawIndexed(const std::shared_ptr<VertexInput>& vertexInput, uint32_t indexCount = 0) = 0;

		static std::unique_ptr<BaseRenderAPI> Create();

		static ERenderAPI GetAPI() { return s_Api; }

	private:
		static ERenderAPI s_Api;
	};
	typedef BaseRenderAPI RenderAPI;
}