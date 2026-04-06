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

        /**
         * @brief Construct a Tilemap with the given chunk layout.
         *
         * @param chunksX Number of chunks in X direction.
         * @param chunksY Number of chunks in Y direction.
         * @param chunkSize Size of each chunk (tiles per side).
         */
        Tilemap(int chunksX, int chunksY, int chunkSize);

        /**
         * @brief Set the sprite atlas used by the tilemap.
         *
         * @param atlas Asset reference to the sprite atlas.
         */
        inline void SetAtlas(AssetRef<SpriteAtlasAsset> atlas) { m_Atlas = atlas; }

        /**
         * @brief Get the sprite atlas asset reference.
         *
         * @return AssetRef to the currently set atlas.
         */
        inline AssetRef<SpriteAtlasAsset> GetAtlas() const { return m_Atlas; }

        inline int GetChunkSize() const { return m_ChunkSize; }


        /**
         * @brief Resize the tilemap and its chunk configuration.
         *
         * @param newChunksX New number of chunks in X.
         * @param newChunksY New number of chunks in Y.
         * @param newChunkSize New chunk size in tiles.
         */
        void Resize(int newChunksX, int newChunksY, int newChunkSize);

        /**
         * @brief Set the tile id at world tile coordinates.
         *
         * @param x Tile X coordinate.
         * @param y Tile Y coordinate.
         * @param tileId ID of the tile to set.
         */
        void SetTile(int x, int y, int tileId);

        /**
         * @brief Get the tile id at world tile coordinates.
         *
         * @param x Tile X coordinate.
         * @param y Tile Y coordinate.
         * @return Tile id at the given location.
         */
        int  GetTile(int x, int y) const;

        /**
         * @brief Check whether the given tile coordinates are within the map bounds.
         *
         * @param x Tile X coordinate.
         * @param y Tile Y coordinate.
         * @return true if the coordinates are valid, false otherwise.
         */
        bool IsValidTile(int x, int y) const;

        /**
         * @brief Set a tile within a specific chunk.
         */
        void SetTile(int chunkX, int chunkY, int x, int y, int tileId);

        /**
         * @brief Get a tile id from a specific chunk.
         */
        int  GetTile(int chunkX, int chunkY, int x, int y) const;

        /**
         * @brief Rebuild any chunks marked as dirty.
         */
        void RebuildDirtyChunks();

        /**
         * @brief Access the internal chunk array.
         *
         * @return Const reference to the vector of TilemapChunk.
         */
        inline const std::vector<TilemapChunk>& GetChunks() const { return m_Chunks; }


        /**
         * @brief Set the unit size in pixels used for tile rendering.
         *
         * Marks the tilemap as dirty.
         */
        inline void SetUnitSize(float pixels) { m_UnitSize = pixels; m_IsDirty = true; }

        /**
         * @brief Get the unit size in pixels.
         */
        inline float GetUnitSize() const { return m_UnitSize; }

        /**
         * @brief Set the number of chunks in X direction and mark dirty.
         */
        inline void SetChunksX(float chunks) { m_ChunksX = chunks; m_IsDirty = true; }

        /**
         * @brief Set the number of chunks in Y direction and mark dirty.
         */
        inline void SetChunksY(float chunks) { m_ChunksY = chunks; m_IsDirty = true; }

        /**
         * @brief Get the number of chunks in X direction.
         */
        inline float GetChunksX() const { return m_ChunksX; }

        /**
         * @brief Get the number of chunks in Y direction.
         */
        inline float GetChunksY() const { return m_ChunksY; }

    private:
        void BuildChunk(TilemapChunk& chunk);

    private:
        float m_UnitSize = 0.5f;
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