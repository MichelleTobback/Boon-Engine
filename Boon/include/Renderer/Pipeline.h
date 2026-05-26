#pragma once

#include "Renderer/Shader.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/Renderer.h"

#include <Renderer/RendererTypes.h>

#include <memory>

namespace Boon
{
	struct PipelineDescriptor
	{
		std::shared_ptr<Shader> Shader = nullptr;
		VertexBufferLayout Layout;

		PrimitiveType Primitive = PrimitiveType::Triangles;

		BlendMode Blend = BlendMode::Alpha;
		DepthMode Depth = DepthMode::ReadWrite;
		CullMode Cull = CullMode::Back;
	};

	class Pipeline
	{
	public:
		explicit Pipeline(const PipelineDescriptor& desc);
		~Pipeline();

		void Bind();
		void Unbind();

		const PipelineDescriptor& GetDescriptor() const;
		std::shared_ptr<Shader> GetShader() const;

		static std::shared_ptr<Pipeline> Create(const PipelineDescriptor& desc);

	protected:
		PipelineDescriptor m_Desc;

		struct Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}