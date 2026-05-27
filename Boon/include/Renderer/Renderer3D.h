#pragma once

#include "Renderer/Material.h"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace Boon
{
	class VertexInput;
	struct RenderContext;

	struct GeometryRenderItem3D
	{
		glm::mat4 Transform{ 1.0f };

		std::shared_ptr<VertexInput> VertexInput = nullptr;
		std::shared_ptr<Material> Material = nullptr;

		int EntityID = -1;

		int SortLayer = 0;
		float SortOrder = 0.0f;
	};

	class Renderer3D final
	{
	public:
		Renderer3D() = default;
		~Renderer3D() = default;

		void Begin(RenderContext& context);
		void End(RenderContext& context);

		void SubmitGeometry(const GeometryRenderItem3D& item);

	private:
		void Flush(RenderContext& context);

	private:
		std::vector<GeometryRenderItem3D> m_GeometryQueue;
	};
}