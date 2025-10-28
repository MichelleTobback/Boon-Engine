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
	};
}