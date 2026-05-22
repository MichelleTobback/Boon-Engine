#pragma once

#include "Panels/AssetEditor.h"
#include "UI/EditorCanvas2D.h"

#include <Asset/TilemapAsset.h>
#include <Asset/SpriteAtlasAsset.h>

#include <glm/glm.hpp>

#include <vector>
#include <queue>
#include <set>

using namespace Boon;

namespace BoonEditor
{
    enum class TileBrushMode
    {
        Paint,
        Erase,
        Fill,
        Random,
        Eyedropper
    };

    enum class TileBrushShape
    {
        Square,
        Circle
    };

    class TilemapEditorPanel final : public AssetEditor<TilemapAsset>
    {
    public:
        TilemapEditorPanel(const std::string& name, EditorContext* pContext);

        virtual void Update() override;
        virtual void OnViewportCanvasRenderUI(const ViewportCanvasContext& context) override;

    protected:
        virtual void RenderToolbar() override;
        virtual void RenderMainArea() override;

    private:
        void RenderToolSettings(Tilemap& tilemap);
        void RenderPalette(Tilemap& tilemap);
        void RenderRandomBrush(Tilemap& tilemap);

        void RenderViewportTilemap(Tilemap& tilemap);
        void RenderTiles(Tilemap& tilemap);
        void RenderTilemapGrid(Tilemap& tilemap);
        void RenderChunkGrid(Tilemap& tilemap);
        void RenderBrushPreview(Tilemap& tilemap, const glm::ivec2& tile);

        void HandlePainting(Tilemap& tilemap, const glm::ivec2& tile);
        void PaintBrush(Tilemap& tilemap, const glm::ivec2& tile, int tileId);
        void FloodFill(Tilemap& tilemap, int x, int y, int newTile);
        int SelectRandomTile() const;

        bool GetAtlasInfo(Tilemap& tilemap, std::shared_ptr<SpriteAtlas>& outAtlas, ImTextureID& outTextureId) const;

        static ImVec2 UV0(const SpriteFrame& frame);
        static ImVec2 UV1(const SpriteFrame& frame);

        void RenderAtlasPalette(Tilemap& tilemap);
        int HitTestAtlasFrame(SpriteAtlas& atlas, const glm::vec2& pixelPos, const glm::vec2& textureSize) const;

        static glm::vec2 UVToPixel(const SpriteFrame& frame, const glm::vec2& textureSize);
        static glm::vec2 SizeToPixel(const SpriteFrame& frame, const glm::vec2& textureSize);

    private:
        EditorCanvas2D m_TilemapCanvas;
        EditorCanvas2D m_PaletteCanvas;

        TileBrushMode m_Mode = TileBrushMode::Paint;
        TileBrushShape m_BrushShape = TileBrushShape::Square;

        int m_SelectedTile = -1;
        int m_BrushSize = 1;

        bool m_ShowGrid = true;
        bool m_ShowChunks = true;
        bool m_ShowEmptyTiles = false;

        std::vector<int> m_RandomBrushTiles;
        std::vector<float> m_RandomBrushWeights;
    };
}