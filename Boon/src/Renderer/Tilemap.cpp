#include "Renderer/Tilemap.h"
#include "Renderer/Renderer.h"
#include "Renderer/VertexData.h"
#include "Core/Random.h"
#include <glm/glm.hpp>

namespace Boon
{
    Tilemap::Tilemap(int chunksX, int chunksY, int chunkSize)
    {
        m_ChunkSize = chunkSize;
        m_ChunksX = chunksX;
        m_ChunksY = chunksY;

        m_MapWidth = chunksX * chunkSize;
        m_MapHeight = chunksY * chunkSize;

        m_Chunks.reserve(m_ChunksX * m_ChunksY);

        VertexBufferLayout quadBufferLayout = {
        { ShaderDataType::Float3, "a_Position"	   },
        { ShaderDataType::Float4, "a_Color"		   },
        { ShaderDataType::Float2, "a_TexCoord"     }
        };

        for (int cy = 0; cy < m_ChunksY; cy++)
        {
            for (int cx = 0; cx < m_ChunksX; cx++)
            {
                TilemapChunk chunk{};
                chunk.ChunkX = cx;
                chunk.ChunkY = cy;
                chunk.Dirty = true;

                // Allocate tile data
                chunk.Tiles.resize(m_ChunkSize * m_ChunkSize, -1);

                // Allocate GPU buffers
                chunk.VertexInput = VertexInput::Create();
                chunk.VertexBuffer = VertexBuffer::Create(sizeof(TileVertex) * m_ChunkSize * m_ChunkSize * 4);
                chunk.VertexBuffer->SetLayout(quadBufferLayout);

                auto indexBuffer = IndexBuffer::Create(nullptr, m_ChunkSize * m_ChunkSize * 6);

                chunk.VertexInput->SetIndexBuffer(indexBuffer);
                chunk.VertexInput->AddVertexBuffer(chunk.VertexBuffer);

                m_Chunks.push_back(std::move(chunk));
            }
        }
    }



    void Tilemap::Resize(int newChunksX, int newChunksY, int newChunkSize)
    {
        int oldChunksX = m_ChunksX;
        int oldChunksY = m_ChunksY;
        int oldChunkSize = m_ChunkSize;

        m_ChunksX = newChunksX;
        m_ChunksY = newChunksY;
        m_ChunkSize = newChunkSize;

        m_MapWidth = m_ChunksX * m_ChunkSize;
        m_MapHeight = m_ChunksY * m_ChunkSize;

        std::vector<TilemapChunk> newChunks;
        newChunks.resize(m_ChunksX * m_ChunksY);

        // Initialize metadata for new chunks
        for (int cy = 0; cy < m_ChunksY; cy++)
        {
            for (int cx = 0; cx < m_ChunksX; cx++)
            {
                TilemapChunk& chunk = newChunks[cy * m_ChunksX + cx];
                chunk.ChunkX = cx;
                chunk.ChunkY = cy;
                chunk.Tiles.resize(m_ChunkSize * m_ChunkSize, -1);
                chunk.Dirty = true;

                // Setup GPU buffers
                chunk.VertexInput = VertexInput::Create();
                chunk.VertexBuffer = VertexBuffer::Create(
                    sizeof(TileVertex) * m_ChunkSize * m_ChunkSize * 4
                );

                auto indexBuffer = IndexBuffer::Create(
                    nullptr,
                    m_ChunkSize * m_ChunkSize * 6
                );

                chunk.VertexInput->SetIndexBuffer(indexBuffer);
                chunk.VertexInput->AddVertexBuffer(chunk.VertexBuffer);
            }
        }

        // Copy tile data from old layout to new
        for (int cy = 0; cy < std::min(oldChunksY, newChunksY); cy++)
        {
            for (int cx = 0; cx < std::min(oldChunksX, newChunksX); cx++)
            {
                const TilemapChunk& oldChunk = m_Chunks[cy * oldChunksX + cx];
                TilemapChunk& newChunk = newChunks[cy * m_ChunksX + cx];

                int copyWidth = std::min(oldChunkSize, newChunkSize);
                int copyHeight = std::min(oldChunkSize, newChunkSize);

                for (int y = 0; y < copyHeight; y++)
                {
                    for (int x = 0; x < copyWidth; x++)
                    {
                        newChunk.Tiles[y * newChunkSize + x] =
                            oldChunk.Tiles[y * oldChunkSize + x];
                    }
                }
            }
        }

        // Replace chunk list
        m_Chunks = std::move(newChunks);
    }


    void Tilemap::SetTile(int x, int y, int tileId)
    {
        int cx = x / m_ChunkSize;
        int cy = y / m_ChunkSize;
        int lx = x % m_ChunkSize;
        int ly = y % m_ChunkSize;

        int idx = cy * m_ChunksX + cx;
        TilemapChunk& chunk = m_Chunks[idx];

        chunk.Tiles[ly * m_ChunkSize + lx] = tileId;
        chunk.Dirty = true;
        m_IsDirty = true;
    }

    int Tilemap::GetTile(int x, int y) const
    {
        if (x < 0 || y < 0 || x >= m_MapWidth || y >= m_MapHeight)
            return -1;

        int cx = x / m_ChunkSize;
        int cy = y / m_ChunkSize;
        int lx = x % m_ChunkSize;
        int ly = y % m_ChunkSize;

        int idx = cy * m_ChunksX + cx;
        return m_Chunks[idx].Tiles[ly * m_ChunkSize + lx];
    }

    bool Tilemap::IsValidTile(int x, int y) const
    {
        if (x < 0 || y < 0 || x >= m_MapWidth || y >= m_MapHeight)
            return false;

        return true;
    }

    void Tilemap::SetTile(int chunkX, int chunkY, int x, int y, int tileId)
    {
        int idx = chunkY * m_ChunksX + chunkX;
        TilemapChunk& chunk = m_Chunks[idx];

        chunk.Tiles[y * m_ChunkSize + x] = tileId;
        chunk.Dirty = true;
        m_IsDirty = true;
    }

    int Tilemap::GetTile(int chunkX, int chunkY, int x, int y) const
    {
        if (chunkX < 0 || chunkY < 0 || chunkX >= m_ChunksX || chunkY >= m_ChunksY)
            return -1;

        if (x < 0 || y < 0 || x >= m_ChunkSize || y >= m_ChunkSize)
            return -1;

        int idx = chunkY * m_ChunksX + chunkX;
        int idy = y * m_ChunkSize + x;
        return m_Chunks[idx].Tiles[idy];
    }

    void Tilemap::RebuildDirtyChunks()
    {
        if (!m_Atlas.IsValid())
            return;

        if (!m_IsDirty)
            return;

        for (TilemapChunk& chunk : m_Chunks)
        {
            if (chunk.Dirty)
                BuildChunk(chunk);
        }

        m_IsDirty = false;
    }

    void Tilemap::BuildChunk(TilemapChunk& chunk)
    {
        auto atlas = m_Atlas->GetInstance();
        auto texture = atlas->GetTexture().Instance();

        std::vector<TileVertex> verts;
        std::vector<uint32_t> indices;

        verts.reserve(m_ChunkSize * m_ChunkSize * 4);
        indices.reserve(m_ChunkSize * m_ChunkSize * 6);

        uint32_t indexOffset = 0;

        const float ppu = 32.f;

        for (int ty = 0; ty < m_ChunkSize; ty++)
        {
            for (int tx = 0; tx < m_ChunkSize; tx++)
            {
                int tileId = chunk.Tiles[ty * m_ChunkSize + tx];
                if (tileId < 0)
                    continue;

                const SpriteFrame& f = atlas->GetSpriteFrame(tileId);

                float width = m_UnitSize;
                float height = m_UnitSize;

                // World-space tile offset inside the chunk (top-left anchored)
                float worldX = (chunk.ChunkX * m_ChunkSize * width) + tx * width;
                float worldY = (chunk.ChunkY * m_ChunkSize * height) + ty * height;

                // Convert to centered quad to match Renderer2D
                float cx = worldX + width * 0.5f;
                float cy = worldY + height * 0.5f;

                float depth = -0.01f;
                glm::vec3 p0 = { cx - width * 0.5f, cy - height * 0.5f, depth };
                glm::vec3 p1 = { cx + width * 0.5f, cy - height * 0.5f, depth };
                glm::vec3 p2 = { cx + width * 0.5f, cy + height * 0.5f, depth };
                glm::vec3 p3 = { cx - width * 0.5f, cy + height * 0.5f, depth };

                // Half-texel padding
                float epsilon = 0.0005f;
                float texelX = epsilon;
                float texelY = epsilon;
                //float texelX = 1.f / texture->GetWidth();
                //float texelY = 1.f / texture->GetHeight();
                
                glm::vec2 uv0 = f.UV + glm::vec2(texelX, texelY);
                glm::vec2 uv1 = f.UV + glm::vec2(f.Size.x - texelX, texelY);
                glm::vec2 uv2 = f.UV + glm::vec2(f.Size.x - texelX, f.Size.y - texelY);
                glm::vec2 uv3 = f.UV + glm::vec2(texelX, f.Size.y - texelY);

                //glm::vec2 uv0 = f.UV;
                //glm::vec2 uv1 = { f.UV.x + f.Size.x, f.UV.y };
                //glm::vec2 uv2 = { f.UV.x + f.Size.x, f.UV.y + f.Size.y };
                //glm::vec2 uv3 = { f.UV.x,            f.UV.y + f.Size.y };

                glm::vec4 color = { 1,1,1,1 };

                verts.push_back(TileVertex{ p0, color, uv0 });
                verts.push_back(TileVertex{ p1, color, uv1 });
                verts.push_back(TileVertex{ p2, color, uv2 });
                verts.push_back(TileVertex{ p3, color, uv3 });

                indices.push_back(indexOffset + 0);
                indices.push_back(indexOffset + 1);
                indices.push_back(indexOffset + 2);
                indices.push_back(indexOffset + 2);
                indices.push_back(indexOffset + 3);
                indices.push_back(indexOffset + 0);

                indexOffset += 4;
            }
        }

        chunk.VertexBuffer->SetData(verts.data(), verts.size() * sizeof(TileVertex));

        auto indexBuffer = IndexBuffer::Create(indices.data(), indices.size());
        chunk.VertexInput->SetIndexBuffer(indexBuffer);

        chunk.Dirty = false;
    }
}
