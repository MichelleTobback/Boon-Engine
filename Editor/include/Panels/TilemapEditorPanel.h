#pragma once
#include "Panels/AssetEditor.h"
#include "Panels/EditorPanel.h"
#include "Renderer/SceneRenderer.h"
#include "Core/EditorCamera.h"

#include <Asset/TilemapAsset.h>
#include <Scene/Scene.h>

#include <vector>
#include <glm/glm.hpp>

namespace BoonEditor
{
    enum class TileBrushShape
    {
        Square,
        Circle
    };

    enum class TileBrushMode
    {
        Paint,
        Erase,
        FloodFill,
        Stamp,
        Random
    };

    class ViewportPanel;

    class TilemapEditorPanel : public AssetEditor<TilemapAsset>
    {
    public:
        TilemapEditorPanel(const std::string& name, DragDropRouter* pRouter);

        virtual void Update() override;

    protected:
        virtual void BuildPreviewScene(Scene& scene) override;
        virtual void RenderToolbar() override;
        virtual void RenderMainArea() override;

    private:
        void RenderPalette(const ImVec2& size);
        void HandlePainting();

        int SelectRandomTile() const;
        void FloodFill(int x, int y, int newTile);

        // Painting
        int m_SelectedTile = -1;
        TileBrushMode m_Mode = TileBrushMode::Paint;
        TileBrushShape m_BrushShape = TileBrushShape::Square;

        // Square brush
        int m_BrushSize = 1;

        // Random brush
        std::vector<int> m_RandomBrushTiles;
        std::vector<float> m_RandomBrushWeights;

        // Stamping
        bool m_StampSelecting = false;
        glm::ivec2 m_StampStart;
        std::vector<int> m_StampBuffer;
        int m_StampW = 0;
        int m_StampH = 0;
    };
}
