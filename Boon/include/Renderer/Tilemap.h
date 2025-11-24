#pragma once
#include "Reflection/BClassBase.h"
#include "SpriteAtlas.h"
#include "Renderer/VertexInput.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/IndexBuffer.h"
#include "Asset/SpriteAtlasAsset.h"

#include <vector>
#include <memory>

namespace Boon
{
    struct TilemapChunk
    {
        int ChunkX, ChunkY;
        bool Dirty = true;

        std::vector<int> Tiles;

        std::shared_ptr<VertexInput>  VertexInput;
        std::shared_ptr<VertexBuffer> VertexBuffer;
    };

    class Tilemap
    {
    public:
        Tilemap() = default;
        Tilemap(int chunksX, int chunksY, int chunkSize);

        inline void SetAtlas(AssetRef<SpriteAtlasAsset> atlas) { m_Atlas = atlas; }
        inline AssetRef<SpriteAtlasAsset> GetAtlas() const { return m_Atlas; }

        inline int GetChunkSize() const { return m_ChunkSize; }

        void Resize(int newChunksX, int newChunksY, int newChunkSize);

        void SetTile(int x, int y, int tileId);
        int  GetTile(int x, int y) const;
        bool IsValidTile(int x, int y) const;

        void SetTile(int chunkX, int chunkY, int x, int y, int tileId);
        int  GetTile(int chunkX, int chunkY, int x, int y) const;

        void RebuildDirtyChunks();
        inline const std::vector<TilemapChunk>& GetChunks() const { return m_Chunks; }

        inline void SetUnitSize(float pixels) { m_UnitSize = pixels; m_IsDirty = true; }
        inline float GetUnitSize() const { return m_UnitSize; }

        inline void SetChunksX(float chunks) { m_ChunksX = chunks; m_IsDirty = true; }
        inline void SetChunksY(float chunks) { m_ChunksY = chunks; m_IsDirty = true; }
        inline float GetChunksX() const { return m_ChunksX; }
        inline float GetChunksY() const { return m_ChunksY; }

    private:
        void BuildChunk(TilemapChunk& chunk);

    private:
        float m_UnitSize = 1.0f;
        int m_MapWidth = 0;
        int m_MapHeight = 0;
        int m_ChunkSize = 16;

        int m_ChunksX = 0;
        int m_ChunksY = 0;

        bool m_IsDirty = true;

        std::vector<TilemapChunk> m_Chunks;

        AssetRef<SpriteAtlasAsset> m_Atlas;
    };
}