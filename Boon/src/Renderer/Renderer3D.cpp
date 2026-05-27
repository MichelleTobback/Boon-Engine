#include "Renderer/Renderer3D.h"

#include "Renderer/RenderContext.h"
#include "Renderer/Renderer.h"
#include "Renderer/UniformBuffer.h"
#include "Renderer/UBData.h"
#include "Renderer/VertexInput.h"

#include <algorithm>

namespace Boon
{
	void Renderer3D::Begin(RenderContext&)
	{
		m_GeometryQueue.clear();
	}

	void Renderer3D::End(RenderContext& context)
	{
		Flush(context);
	}

	void Renderer3D::SubmitGeometry(const GeometryRenderItem3D& item)
	{
		m_GeometryQueue.push_back(item);
	}

	void Renderer3D::Flush(RenderContext& context)
	{
		std::sort(m_GeometryQueue.begin(), m_GeometryQueue.end(),
			[](const GeometryRenderItem3D& a, const GeometryRenderItem3D& b)
			{
				if (a.SortLayer != b.SortLayer)
					return a.SortLayer < b.SortLayer;

				return a.SortOrder < b.SortOrder;
			});

		for (const GeometryRenderItem3D& item : m_GeometryQueue)
		{
			if (!item.VertexInput || !item.Material)
				continue;

			UBData::Object objectData{};
			objectData.World = item.Transform;
			objectData.ID = item.EntityID;
			context.ObjectUniformBuffer.SetValue(objectData);

			item.Material->Bind();

			Renderer::DrawIndexed(item.VertexInput);

			item.Material->Unbind();
		}

		m_GeometryQueue.clear();
	}
}