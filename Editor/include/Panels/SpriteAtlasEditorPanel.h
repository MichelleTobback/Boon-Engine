#pragma once
#include "Panels/EditorPanel.h"

#include <Asset/SpriteAtlasAsset.h>
#include <Scene/Scene.h>

#include <vector>
#include <set>
#include <glm/glm.hpp>

using namespace Boon;

namespace BoonEditor
{

    class ViewportPanel;

    enum class AtlasEditorMode
    {
        Select, Create
    };

    enum class GridMode
    {
        Cellsize, RowCols
    };

    class SpriteAtlasEditorPanel : public EditorPanel
    {
    public:
        SpriteAtlasEditorPanel(const std::string& name,
            DragDropRouter* pRouter,
            AssetRef<SpriteAtlasAsset> asset);

        virtual void Update() override;
        virtual void OnRenderUI() override;

        inline Scene* GetScene() const { return m_pScene; }
        inline void SetViewport(ViewportPanel* pViewport) { m_pViewport = pViewport; }

        inline AssetRef<SpriteAtlasAsset> GetAsset() const { return m_Asset; }
        inline void SetAsset(AssetRef<SpriteAtlasAsset> asset) { m_Asset = asset; }

    private:
        void RenderToolbar();
        void RenderMainArea();

        glm::vec3 ScreenToWorld(const glm::vec2& mousePos);
        glm::vec2 CameraWorldToAtlas(const glm::vec3& world);

    private:
        AssetRef<SpriteAtlasAsset> m_Asset;

        Scene* m_pScene{ nullptr };
        ViewportPanel* m_pViewport = nullptr;

        int m_SelectedSprite = -1;

        AtlasEditorMode m_Mode{ AtlasEditorMode::Select };
        GridMode m_GridMode{ GridMode::Cellsize };
        glm::ivec2 m_GridTileSize = { 32, 32 };
        int m_Cols{ 4 }, m_Rows{ 4 };
        std::set<int> m_SelectedTiles;
    };
}
