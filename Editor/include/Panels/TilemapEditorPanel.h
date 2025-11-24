#pragma once
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

    class TilemapEditorPanel : public EditorPanel
    {
    public:
        TilemapEditorPanel(const std::string& name,
            DragDropRouter* pRouter,
            AssetRef<TilemapAsset> asset);

        virtual void Update() override;
        virtual void OnRenderUI() override;

        inline Scene* GetScene() const { return m_pScene; }
        inline void SetViewport(ViewportPanel* pViewport) { m_pViewport = pViewport; }

        inline AssetRef<TilemapAsset> GetAsset() const { return m_Asset; }
        inline void SetAsset(AssetRef<TilemapAsset> asset) { m_Asset = asset; }

    private:
        void RenderToolbar();
        void RenderMainArea();
        void RenderPalette(const ImVec2& size);
        void HandlePainting();

        glm::vec3 ScreenToWorld(const glm::vec2& mousePos);

        int SelectRandomTile() const;
        void FloodFill(int x, int y, int newTile);

    private:
        AssetRef<TilemapAsset> m_Asset;

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

        Scene* m_pScene{ nullptr };
        ViewportPanel* m_pViewport = nullptr;
    };
}
