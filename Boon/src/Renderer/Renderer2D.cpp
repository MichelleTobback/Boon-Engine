#include "Renderer/Renderer2D.h"
#include "Renderer/VertexInput.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"
#include "Renderer/VertexData.h"

#include "Core/ServiceLocator.h"

#include "Asset/AssetLibrary.h"
#include "Asset/ShaderAsset.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Boon
{
	static const uint32_t s_MaxQuads = 20000;
	static const uint32_t s_MaxLines = s_MaxQuads * 2;
	static const uint32_t s_MaxVertices = s_MaxQuads * 4;
	static const uint32_t s_MaxIndices = s_MaxQuads * 6;
	static const uint32_t s_MaxTextureSlots = 32;
}

using namespace Boon;

Boon::Renderer2D::Renderer2D()
{
	m_QuadVertexPositions[0] = { -0.5f, -0.5f, 0.f, 1.f, };
	m_QuadVertexPositions[1] = { 0.5f, -0.5f, 0.f, 1.f, };
	m_QuadVertexPositions[2] = { 0.5f,  0.5f, 0.f, 1.f, };
	m_QuadVertexPositions[3] = { -0.5f,  0.5f, 0.f, 1.f };

	m_TextureSlots[0] = Texture2D::Create(TextureDescriptor());
	uint32_t whiteTextureData = 0xffffffff;
	m_TextureSlots[0]->SetData(&whiteTextureData, sizeof(uint32_t));

	//shaders
	AssetLibrary& assets{ ServiceLocator::Get<AssetLibrary>() };

	AssetHandle lineShaderHandle = assets.Load<ShaderAssetLoader>("shaders/Line.glsl");
	auto pLineShader = assets.GetAsset<ShaderAsset>(lineShaderHandle);

	AssetHandle quadHandle = assets.Load<ShaderAssetLoader>("shaders/Quad.glsl");
	auto pSpriteShader = assets.GetAsset<ShaderAsset>(quadHandle);

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
	auto quadIndexBuffer = IndexBuffer::Create(quadIndices, s_MaxIndices);
	m_QuadBatch.Initialize(s_MaxVertices, quadBufferLayout, pSpriteShader, PrimitiveType::Triangles, quadIndexBuffer);
	delete[] quadIndices;
	m_QuadBatch.BindBeginBatchCallback([this]() {
		m_TextureSlotIndex = 1;
		});
	m_QuadBatch.BindPreFlushCallback([this]() {
		for (uint32_t i = 0; i < m_TextureSlotIndex; i++)
			m_TextureSlots[i]->Bind(i);
		});

	//lines
	VertexBufferLayout lineBufferLayout = {
		{ ShaderDataType::Float3, "a_Position"	},
		{ ShaderDataType::Float4, "a_Color"		}
	};
	m_LineBatch.Initialize(s_MaxVertices, lineBufferLayout, pLineShader, PrimitiveType::Lines);
}

Boon::Renderer2D::~Renderer2D()
{
}

void Boon::Renderer2D::Begin()
{
	m_QuadBatch.Begin();
	m_LineBatch.Begin();
}

void Boon::Renderer2D::End()
{
	m_QuadBatch.Flush();
	m_LineBatch.Flush();
}

void Boon::Renderer2D::SubmitQuad(const glm::mat4& transform, const glm::vec4& color, int gameObjectHandle)
{
	constexpr size_t quadVertexCount = 4;
	const float textureIndex = 0.0f;
	constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
	const float tilingFactor = 1.0f;

	for (size_t i = 0; i < quadVertexCount; i++)
	{
		QuadVertex& vertex = m_QuadBatch.PushVertex<QuadVertex>();
		vertex.Position = transform * m_QuadVertexPositions[i];
		vertex.Color = color;
		vertex.TexCoord = textureCoords[i];
		vertex.TexIndex = textureIndex;
		vertex.TilingFactor = tilingFactor;
		vertex.GameObjectID = gameObjectHandle;
	}
}

void Boon::Renderer2D::SubmitQuad(const glm::mat4& transform, const std::shared_ptr<Texture2D>& texture, float tilingFactor, const glm::vec4& color, int gameObjectHandle)
{
	SubmitQuad(transform, texture, tilingFactor, color, gameObjectHandle, { 0.f, 0.f }, { 1.f, 1.f });
}

void Boon::Renderer2D::SubmitQuad(const glm::mat4& transform, const std::shared_ptr<Texture2D>& texture, float tilingFactor,
	const glm::vec4& color, int gameObjectHandle, const glm::vec2& spriteTexCoord, const glm::vec2& spriteTexSize)
{
	constexpr size_t quadVertexCount = 4;

	glm::vec2 texCoords[quadVertexCount] = {
		{ spriteTexCoord.x,                    spriteTexCoord.y },                     // bottom-left
		{ spriteTexCoord.x + spriteTexSize.x,  spriteTexCoord.y },                     // bottom-right
		{ spriteTexCoord.x + spriteTexSize.x,  spriteTexCoord.y + spriteTexSize.y },   // top-right
		{ spriteTexCoord.x,                    spriteTexCoord.y + spriteTexSize.y }    // top-left
	};

	float textureIndex = 0.0f;
	bool found = false;
	for (uint32_t i = 1; i < m_TextureSlotIndex; i++)
	{
		if (m_TextureSlots[i]->GetRendererID() == texture->GetRendererID())
		{
			textureIndex = (float)i;
			found = true;
			break;
		}
	}
	if (!found)
	{
		if (m_TextureSlotIndex >= s_MaxTextureSlots)
			m_QuadBatch.NextBatch();

		textureIndex = (float)m_TextureSlotIndex;
		m_TextureSlots[m_TextureSlotIndex] = texture;
		m_TextureSlotIndex++;
	}

	float ppu = 32.0f;
	glm::vec2 pixelSize = spriteTexSize * glm::vec2(texture->GetWidth(), texture->GetHeight());
	glm::vec2 worldSize = pixelSize / ppu;

	glm::mat4 scaledTransform = glm::scale(transform, glm::vec3(worldSize.x, worldSize.y, 1.0f));

	for (size_t i = 0; i < quadVertexCount; i++)
	{
		QuadVertex& vertex = m_QuadBatch.PushVertex<QuadVertex>();
		vertex.Position = scaledTransform * m_QuadVertexPositions[i];
		vertex.Color = color;
		vertex.TexCoord = texCoords[i];
		vertex.TexIndex = textureIndex;
		vertex.TilingFactor = tilingFactor;
		vertex.GameObjectID = gameObjectHandle;
	}
}

void Boon::Renderer2D::SubmitLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color)
{
	LineVertex& vertex0 = m_LineBatch.PushVertex<LineVertex>();
	vertex0.Position = p0;
	vertex0.Color = color;

	LineVertex& vertex1 = m_LineBatch.PushVertex<LineVertex>();
	vertex1.Position = p1;
	vertex1.Color = color;
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
