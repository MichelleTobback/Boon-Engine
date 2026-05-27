#pragma once

#include "Panels/AssetEditor.h"
#include "UI/EditorCanvas2D.h"

#include <Asset/SpriteAtlasAsset.h>

#include <glm/glm.hpp>

#include <set>
#include <string>
#include <vector>

using namespace Boon;

namespace BoonEditor
{
    enum class AtlasEditorMode
    {
        Select,
        Slice
    };

    enum class GridMode
    {
        Cellsize,
        RowCols
    };

    class SpriteAtlasEditorPanel : public AssetEditor<SpriteAtlasAsset>
    {
    public:
        SpriteAtlasEditorPanel(EditorContext* pContext, const std::string& name);

        virtual void Update() override;

        virtual bool OnViewportCanvasRenderUI(const ViewportCanvasContext& context) override;

    protected:
        virtual void RenderToolbar() override;
        virtual void RenderMainArea() override;

    private:
        void RenderAnimationPreview(SpriteAtlas& atlas);
        void RenderAtlasCanvas(SpriteAtlas& atlas);
        void RenderInspector(SpriteAtlas& atlas);
        void RenderClipEditor(SpriteAtlas& atlas);
        void RenderTimeline(SpriteAtlas& atlas);

        void RenderExistingFrames(SpriteAtlas& atlas);
        void RenderSliceGrid(SpriteAtlas& atlas);

        void AddSelectedGridCellsAsFrames(SpriteAtlas& atlas);
        void AddSelectedFrameToCurrentClip(SpriteAtlas& atlas);

        bool GetTextureInfo(SpriteAtlas& atlas, ImTextureID& outTextureId, glm::vec2& outTextureSize) const;

        int HitTestFrame(SpriteAtlas& atlas, const glm::vec2& pixelPos) const;

        static glm::vec2 UVToPixel(const SpriteFrame& frame, const glm::vec2& textureSize);
        static glm::vec2 SizeToPixel(const SpriteFrame& frame, const glm::vec2& textureSize);
        static SpriteFrame PixelToFrame(const glm::vec2& pixelPos, const glm::vec2& pixelSize, const glm::vec2& textureSize);

        void RestartPreview();
        void UpdateAnimationPreview(SpriteAtlas& atlas);
    private:
        EditorCanvas2D m_AtlasCanvas;

        int m_SelectedSprite = -1;
        int m_SelectedClip = -1;

        int m_DropPreviewIndex = -1;

        int m_TimelineDraggedFrame = -1;
        bool m_TimelineDraggingFromAtlas = false;
        int m_TimelinePreviewInsert = -1;

        AtlasEditorMode m_Mode = AtlasEditorMode::Select;
        GridMode m_GridMode = GridMode::Cellsize;

        glm::ivec2 m_GridTileSize{ 32, 32 };
        int m_Cols = 4;
        int m_Rows = 4;

        std::set<int> m_SelectedGridCells;

        int m_PreviewFrame = 0;
        float m_PreviewTimer = 0.0f;
        bool m_PreviewPlaying = true;
        int m_LastPreviewClip = -1;
    };
}