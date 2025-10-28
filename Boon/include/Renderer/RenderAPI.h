#pragma once
#include <memory>

namespace Boon
{
	enum class RenderAPI
	{
		None,
		OpenGL
	};

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

		static std::unique_ptr<BaseRenderAPI> Create();

		static RenderAPI GetAPI() { return s_Api; }

		static RenderAPI s_Api;
	};
}