#pragma once

#include "Asset/ShaderAsset.h"
#include "Renderer/Material.h"
#include "Renderer/Pipeline.h"

#include <memory>

namespace Boon
{
	class MaterialFactory final
	{
	public:
		static std::shared_ptr<Material> CreateFromShaderAsset(
			ShaderAsset& shaderAsset,
			BlendMode blend = BlendMode::Alpha,
			DepthMode depth = DepthMode::ReadWrite,
			CullMode cull = CullMode::None,
			PrimitiveType primitive = PrimitiveType::Triangles)
		{
			auto shader = shaderAsset.GetInstance();
			if (!shader)
				return nullptr;

			PipelineDescriptor desc{};
			desc.Shader = shader;
			desc.Layout = shaderAsset.GetVertexLayout();
			desc.Primitive = primitive;
			desc.Blend = blend;
			desc.Depth = depth;
			desc.Cull = cull;

			auto pipeline = std::make_shared<Pipeline>(desc);

			return std::make_shared<Material>(pipeline, shaderAsset.GetMaterialLayout());
		}
	};
}