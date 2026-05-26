#include "Renderer/Renderer2D.h"
#include "Renderer/VertexInput.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"
#include "Renderer/VertexData.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/UniformBuffer.h"
#include "Renderer/UBData.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace Boon
{
	static const uint32_t s_MaxQuads = 20000;
	static const uint32_t s_MaxLines = s_MaxQuads * 2;
	static const uint32_t s_MaxVertices = s_MaxQuads * 4;
	static const uint32_t s_MaxIndices = s_MaxQuads * 6;
	static const uint32_t s_MaxTextureSlots = 32;

	static float ResolveTextureSlot(
		const std::shared_ptr<Texture2D>& texture,
		std::array<std::shared_ptr<Texture2D>, 32>& textureSlots,
		uint32_t& textureSlotIndex,
		RenderBatch& quadBatch)
	{
		if (!texture)
			return 0.0f;

		for (uint32_t i = 1; i < textureSlotIndex; i++)
		{
			if (textureSlots[i] &&
				textureSlots[i]->GetRendererID() == texture->GetRendererID())
			{
				return static_cast<float>(i);
			}
		}

		if (textureSlotIndex >= 32)
		{
			quadBatch.NextBatch();
		}

		const float index = static_cast<float>(textureSlotIndex);
		textureSlots[textureSlotIndex] = texture;
		textureSlotIndex++;

		return index;
	}
}

using namespace Boon;

Boon::Renderer2D::Renderer2D(const Renderer2DCreateInfo& desc)
{
	m_QuadVertexPositions[0] = { -0.5f, -0.5f, 0.f, 1.f, };
	m_QuadVertexPositions[1] = { 0.5f, -0.5f, 0.f, 1.f, };
	m_QuadVertexPositions[2] = { 0.5f,  0.5f, 0.f, 1.f, };
	m_QuadVertexPositions[3] = { -0.5f,  0.5f, 0.f, 1.f };

	m_pWhiteTexture = Texture2D::Create(TextureDescriptor());
	uint32_t whiteTextureData = 0xffffffff;
	m_pWhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

	//quads
	uint32_t* quadIndices = new uint32_t[s_MaxIndices];
	uint32_t offset = 0;
	for (uint32_t i = 0; i < s_MaxIndices; i += 6)
	{
		quadIndices[i + 0] = offset + 0;
		quadIndices[i + 1] = offset + 1;
		quadIndices[i + 2] = offset + 2;

		quadIndices[i + 3] = offset + 2;
		quadIndices[i + 4] = offset + 3;
		quadIndices[i + 5] = offset + 0;

		offset += 4;
	}

	VertexBufferLayout quadBufferLayout = {
		{ ShaderDataType::Float3, "a_Position"	   },
		{ ShaderDataType::Float4, "a_Color"		   },
		{ ShaderDataType::Float2, "a_TexCoord"     },
		{ ShaderDataType::Float,  "a_TexIndex"     },
		{ ShaderDataType::Float,  "a_TilingFactor" },
		{ ShaderDataType::Int,	  "a_ID"		   }
	};
	m_QuadIndexBuffer = IndexBuffer::Create(quadIndices, s_MaxIndices);

	PipelineDescriptor quadPipelineDesc{};
	quadPipelineDesc.Shader = desc.pSpriteShader;
	quadPipelineDesc.Layout = quadBufferLayout;
	quadPipelineDesc.Primitive = PrimitiveType::Triangles;
	quadPipelineDesc.Blend = BlendMode::Alpha;
	quadPipelineDesc.Depth = DepthMode::ReadWrite;
	quadPipelineDesc.Cull = CullMode::None;
	m_pQuadPipeline = std::make_shared<Pipeline>(quadPipelineDesc);

	//lines
	VertexBufferLayout lineBufferLayout = {
		{ ShaderDataType::Float3, "a_Position"	},
		{ ShaderDataType::Float4, "a_Color"		}
	};

	PipelineDescriptor linePipelineDesc{};
	linePipelineDesc.Shader = desc.pLineShader;
	linePipelineDesc.Layout = lineBufferLayout;
	linePipelineDesc.Primitive = PrimitiveType::Lines;
	linePipelineDesc.Blend = BlendMode::Alpha;
	linePipelineDesc.Depth = DepthMode::ReadWrite;
	linePipelineDesc.Cull = CullMode::None;
	auto linePipeline = std::make_shared<Pipeline>(linePipelineDesc);

	m_LineBatch.Initialize(s_MaxVertices, linePipeline);
}

Boon::Renderer2D::~Renderer2D()
{
}

void Boon::Renderer2D::Begin(RenderContext&)
{
	for (auto& [key, state] : m_QuadBatches)
		state.Batch.Begin();

	m_LineBatch.Begin();
}

void Boon::Renderer2D::End(RenderContext& ctx)
{
	FlushRenderQueue(ctx);

	for (auto& [key, state] : m_QuadBatches)
		state.Batch.Flush();

	m_LineBatch.Flush();
}

void Renderer2D::SubmitQuad(const QuadRenderItem2D& item)
{
	m_RenderQueue.Submit(item);
}

void Boon::Renderer2D::SubmitQuad(const glm::mat4& transform, const std::shared_ptr<Material>& material, int gameObjectHandle)
{
	SubmitQuad(transform, material, gameObjectHandle, { 0.f, 0.f }, { 1.f, 1.f });
}

void Boon::Renderer2D::SubmitQuad(
	const glm::mat4& transform,
	const std::shared_ptr<Material>& material,
	int gameObjectHandle,
	const glm::vec2& spriteTexCoord,
	const glm::vec2& spriteTexSize)
{
	QuadRenderItem2D item{};
	item.Transform = transform;
	item.UV0 = spriteTexCoord;
	item.UV1 = spriteTexCoord + spriteTexSize;
	item.EntityID = gameObjectHandle;
	item.MaterialOverride = material;

	m_RenderQueue.Submit(item);
}

void Boon::Renderer2D::SubmitQuad(
	const glm::mat4& transform,
	const std::shared_ptr<Texture2D>& texture,
	const glm::vec4& color,
	const float tiling,
	int gameObjectHandle,
	const glm::vec2& spriteTexCoord,
	const glm::vec2& spriteTexSize)
{
	QuadRenderItem2D item{};
	item.Transform = transform;
	item.UV0 = spriteTexCoord;
	item.UV1 = spriteTexCoord + spriteTexSize;
	item.EntityID = gameObjectHandle;
	item.Color = color;
	item.Texture = texture;
	item.TilingFactor = tiling;

	m_RenderQueue.Submit(item);
}

void Boon::Renderer2D::SubmitLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color)
{
	LineRenderItem2D item{};
	item.P0 = p0;
	item.P1 = p1;
	item.Color = color;

	m_RenderQueue.Submit(item);
}

void Boon::Renderer2D::SubmitRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
{
	glm::vec3 p0 = position + glm::vec3(m_QuadVertexPositions[0]) * glm::vec3(size.x, size.y, 1.f);
	glm::vec3 p1 = position + glm::vec3(m_QuadVertexPositions[1]) * glm::vec3(size.x, size.y, 1.f);
	glm::vec3 p2 = position + glm::vec3(m_QuadVertexPositions[2]) * glm::vec3(size.x, size.y, 1.f);
	glm::vec3 p3 = position + glm::vec3(m_QuadVertexPositions[3]) * glm::vec3(size.x, size.y, 1.f);

	SubmitLine(p0, p1, color);
	SubmitLine(p1, p2, color);
	SubmitLine(p2, p3, color);
	SubmitLine(p3, p0, color);
}

void Boon::Renderer2D::SubmitRect(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color)
{
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
		* glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f, 0.0f, 1.0f));

	SubmitRect(transform, size, color);
}

void Boon::Renderer2D::SubmitRect(const glm::mat4& transform, const glm::vec2& size, const glm::vec4& color)
{
	glm::vec3 p0 = glm::vec3(transform * (m_QuadVertexPositions[0] * glm::vec4(size.x, size.y, 1.0f, 1.0f)));
	glm::vec3 p1 = glm::vec3(transform * (m_QuadVertexPositions[1] * glm::vec4(size.x, size.y, 1.0f, 1.0f)));
	glm::vec3 p2 = glm::vec3(transform * (m_QuadVertexPositions[2] * glm::vec4(size.x, size.y, 1.0f, 1.0f)));
	glm::vec3 p3 = glm::vec3(transform * (m_QuadVertexPositions[3] * glm::vec4(size.x, size.y, 1.0f, 1.0f)));

	SubmitLine(p0, p1, color);
	SubmitLine(p1, p2, color);
	SubmitLine(p2, p3, color);
	SubmitLine(p3, p0, color);
}

void Boon::Renderer2D::SubmitPolygon(const std::vector<glm::vec3>& positions, const glm::vec4& color)
{
	if (positions.size() < 2)
		return;

	for (size_t i = 0; i < positions.size() - 1; ++i)
	{
		SubmitLine(positions[i], positions[i + 1], color);
	}

	SubmitLine(positions.back(), positions.front(), color);
}

void Boon::Renderer2D::SubmitGeometry(const GeometryRenderItem2D& item)
{
	m_RenderQueue.Submit(item);
}

void Boon::Renderer2D::FlushRenderQueue(RenderContext& ctx)
{
	const auto& geometry = m_RenderQueue.GetGeometry();

	for (const GeometryRenderItem2D& mesh : geometry)
	{
		if (!mesh.VertexInput || !mesh.Material)
			continue;

		auto material = mesh.Material;
		if (!material)
			continue;

		UBData::Object objectData{};
		objectData.World = mesh.Transform;
		objectData.ID = mesh.EntityID;
		ctx.ObjectUniformBuffer.SetValue(objectData);

		material->Bind();

		Renderer::DrawIndexed(mesh.VertexInput);

		material->Unbind();
	}

	constexpr size_t quadVertexCount = 4;

	std::vector<QuadRenderItem2D> quads = m_RenderQueue.GetQuads();
	std::sort(quads.begin(), quads.end(),
		[](const QuadRenderItem2D& a, const QuadRenderItem2D& b)
		{
			auto getPipeline = [](const QuadRenderItem2D& q) -> Pipeline*
				{
					if (q.MaterialOverride && q.MaterialOverride->GetPipeline())
						return q.MaterialOverride->GetPipeline().get();

					return nullptr;
				};

			void* pipelineA = getPipeline(a);
			void* pipelineB = getPipeline(b);

			if (pipelineA != pipelineB)
				return pipelineA < pipelineB;

			if (a.SortLayer != b.SortLayer)
				return a.SortLayer < b.SortLayer;

			return a.SortOrder < b.SortOrder;
		});

	for (const QuadRenderItem2D& quad : quads)
	{
		glm::vec2 texCoords[quadVertexCount] =
		{
			{ quad.UV0.x, quad.UV0.y },
			{ quad.UV1.x, quad.UV0.y },
			{ quad.UV1.x, quad.UV1.y },
			{ quad.UV0.x, quad.UV1.y }
		};

		glm::vec4 color = quad.Color;
		float tilingFactor = quad.TilingFactor;
		std::shared_ptr<Texture2D> texture = quad.Texture;
		std::shared_ptr<Pipeline> pipeline = m_pQuadPipeline;

		if (quad.MaterialOverride && quad.MaterialOverride->GetPipeline())
			pipeline = quad.MaterialOverride->GetPipeline();

		QuadBatchState& batchState = GetOrCreateQuadBatch(pipeline);

		if (quad.MaterialOverride)
		{
			if (const auto* data = quad.MaterialOverride->GetDataAs<QuadMaterialData>())
			{
				color = data->Color;
				tilingFactor = data->TilingFactor;
			}

			if (auto materialTexture = quad.MaterialOverride->GetTexture("u_Texture"))
				texture = materialTexture;
		}

		float textureIndex = ResolveTextureSlot(texture, batchState.TextureSlots, batchState.TextureSlotIndex, batchState.Batch);

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			QuadVertex& vertex = batchState.Batch.PushVertex<QuadVertex>();

			vertex.Position = quad.Transform * m_QuadVertexPositions[i];
			vertex.Color = color;
			vertex.TexCoord = texCoords[i];
			vertex.TexIndex = textureIndex;
			vertex.TilingFactor = tilingFactor;
			vertex.GameObjectID = quad.EntityID;
		}
	}

	const auto& lines = m_RenderQueue.GetLines();

	for (const LineRenderItem2D& line : lines)
	{
		LineVertex& vertex0 = m_LineBatch.PushVertex<LineVertex>();
		vertex0.Position = line.P0;
		vertex0.Color = line.Color;

		LineVertex& vertex1 = m_LineBatch.PushVertex<LineVertex>();
		vertex1.Position = line.P1;
		vertex1.Color = line.Color;
	}

	m_RenderQueue.Clear();
}

Boon::Renderer2D::QuadBatchState& Boon::Renderer2D::GetOrCreateQuadBatch(const std::shared_ptr<Pipeline>& pipeline)
{
	QuadBatchKey key{};
	key.Pipeline = pipeline ? pipeline : m_pQuadPipeline;

	auto it = m_QuadBatches.find(key);
	if (it != m_QuadBatches.end())
		return it->second;

	auto& state = m_QuadBatches[key];

	state.TextureSlots[0] = m_pWhiteTexture;
	state.TextureSlotIndex = 1;

	state.Batch.Initialize(s_MaxVertices, key.Pipeline, m_QuadIndexBuffer);
	state.Batch.Begin();

	state.Batch.BindBeginBatchCallback([&state]()
		{
			state.TextureSlotIndex = 1;
		});

	state.Batch.BindPreFlushCallback([&state]()
		{
			for (uint32_t i = 0; i < state.TextureSlotIndex; i++)
			{
				if (state.TextureSlots[i])
					state.TextureSlots[i]->Bind(i);
			}
		});

	return state;
}
