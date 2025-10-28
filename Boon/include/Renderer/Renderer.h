#pragma once

#include <memory>

namespace Boon
{
	class BaseRenderAPI;
	class Renderer final
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginFrame();
		static void EndFrame();

	private:
		Renderer() = delete;
		static std::unique_ptr<BaseRenderAPI> s_pApi;
	};
}