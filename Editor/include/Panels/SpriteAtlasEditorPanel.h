#pragma once
#include "Panels/AssetEditor.h"

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

    class SpriteAtlasEditorPanel : public AssetEditor<SpriteAtlasAsset>
    {
    public:
        SpriteAtlasEditorPanel(const std::string& name, DragDropRouter* pRouter);

        virtual void Update() override;

    protected:
        virtual void BuildPreviewScene(Scene& scene) override;
        virtual void RenderToolbar() override;
        virtual void RenderMainArea() override;

        glm::vec2 CameraWorldToAtlas(const glm::vec3& world);

    private:
        int m_SelectedSprite = -1;

        AtlasEditorMode m_Mode{ AtlasEditorMode::Select };
        GridMode m_GridMode{ GridMode::Cellsize };
        glm::ivec2 m_GridTileSize = { 32, 32 };
        int m_Cols{ 4 }, m_Rows{ 4 };
        std::set<int> m_SelectedTiles;
    };
}
