#include "Panels/TilemapEditorPanel.h"

#include <imgui.h>

#include "Assets/AssetDatabase.h"
#include "UI/UI.h"
#include "UI/IconsFontAwesome7.h"

#include <Renderer/Texture.h>

#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <ctime>

namespace
{
    ImU32 Col(ImGuiCol idx)
    {
        return ImGui::GetColorU32(ImGui::GetStyleColorVec4(idx));
    }

    ImU32 Accent(float alpha = 1.0f)
    {
        ImVec4 c = ImGui::GetStyleColorVec4(ImGuiCol_CheckMark);
        c.w *= alpha;
        return ImGui::GetColorU32(c);
    }

    ImU32 Surface()
    {
        return Col(ImGuiCol_FrameBg);
    }

    ImU32 SurfaceHover()
    {
        return Col(ImGuiCol_FrameBgHovered);
    }

    ImU32 Border()
    {
        return Col(ImGuiCol_Border);
    }

    ImU32 TextMuted()
    {
        return Col(ImGuiCol_TextDisabled);
    }

    bool EditorButton(const char* label, bool active = false, ImVec2 size = ImVec2(78.0f, 26.0f))
    {
        ImGui::PushStyleColor(ImGuiCol_Button, active ? Accent(0.22f) : Col(ImGuiCol_Button));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, active ? Accent(0.30f) : Col(ImGuiCol_ButtonHovered));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, active ? Accent(0.38f) : Col(ImGuiCol_ButtonActive));

        bool pressed = ImGui::Button(label, size);

        ImGui::PopStyleColor(3);
        return pressed;
    }

    void SectionHeader(const char* title)
    {
        ImGui::Spacing();
        ImGui::TextDisabled("%s", title);
        ImGui::Separator();
    }

    void BeginEditorPanel(const char* id, ImVec2 size = ImVec2(0, 0))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, ImGui::GetStyle().WindowRounding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
        ImGui::BeginChild(id, size, true);
    }

    void EndEditorPanel()
    {
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
    }

    static float FitScale(const glm::vec2& contentSize, const glm::vec2& canvasSize, float padding = 32.0f)
    {
        if (contentSize.x <= 0.0f || contentSize.y <= 0.0f)
            return 1.0f;

        float sx = (canvasSize.x - padding * 2.0f) / contentSize.x;
        float sy = (canvasSize.y - padding * 2.0f) / contentSize.y;

        return std::max(0.05f, std::min(sx, sy));
    }

    bool EditorIconButton(
        const char* icon,
        const char* tooltip,
        bool active = false,
        ImVec2 size = ImVec2(28.0f, 28.0f))
    {
        ImGui::PushStyleColor(
            ImGuiCol_Button,
            active ? Accent(0.22f) : Col(ImGuiCol_Button));

        ImGui::PushStyleColor(
            ImGuiCol_ButtonHovered,
            active ? Accent(0.32f) : Col(ImGuiCol_ButtonHovered));

        ImGui::PushStyleColor(
            ImGuiCol_ButtonActive,
            active ? Accent(0.42f) : Col(ImGuiCol_ButtonActive));

        bool pressed = ImGui::Button(icon, size);

        ImGui::PopStyleColor(3);

        if (ImGui::IsItemHovered() && tooltip)
            ImGui::SetTooltip("%s", tooltip);

        return pressed;
    }
}

namespace BoonEditor
{
    TilemapEditorPanel::TilemapEditorPanel(EditorContext* pContext, const std::string& name)
        : AssetEditor<TilemapAsset>(pContext, name)
    {
        m_TilemapCanvas.SetZoomLimits(0.02f, 128.f);
        m_PaletteCanvas.SetZoomLimits(0.02f, 64.f);
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
    }

    void TilemapEditorPanel::Update()
    {
    }

    void TilemapEditorPanel::OnViewportCanvasRenderUI(const ViewportCanvasContext&)
    {
        if (!m_Asset.IsValid())
            return;

        std::shared_ptr<Tilemap> tilemap = m_Asset->GetInstance();
        if (!tilemap)
            return;

        RenderViewportTilemap(*tilemap);
    }

    void TilemapEditorPanel::RenderToolbar()
    {
        if (!m_Asset.IsValid())
            return;

        constexpr ImVec2 buttonSize{ 30.0f, 30.0f };

        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Tilemap Editor");

        ImGui::SameLine();

        ImGui::TextDisabled("| Atlas palette + brush workflow");

        ImGui::Spacing();

        // ------------------------------------------------------------
        // File
        // ------------------------------------------------------------

        if (EditorIconButton(
            ICON_FA_FLOPPY_DISK,
            "Save",
            false,
            buttonSize))
        {
            AssetDatabase::Get().Export<TilemapAsset>(m_Asset);
        }

        ImGui::SameLine(0.0f, 4.0f);

        if (EditorIconButton(
            ICON_FA_ROTATE_LEFT,
            "Undo",
            false,
            buttonSize))
        {
            if (auto tilemap = m_Asset->GetInstance())
                Undo(*tilemap);
        }

        ImGui::SameLine(0.0f, 4.0f);

        if (EditorIconButton(
            ICON_FA_ROTATE_RIGHT,
            "Redo",
            false,
            buttonSize))
        {
            if (auto tilemap = m_Asset->GetInstance())
                Redo(*tilemap);
        }

        ImGui::SameLine(0.0f, 10.0f);

        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

        ImGui::SameLine(0.0f, 10.0f);

        // ------------------------------------------------------------
        // Brush modes
        // ------------------------------------------------------------

        if (EditorIconButton(
            ICON_FA_BRUSH,
            "Paint (P)",
            m_Mode == TileBrushMode::Paint,
            buttonSize))
        {
            m_Mode = TileBrushMode::Paint;
        }

        ImGui::SameLine(0.0f, 4.0f);

        if (EditorIconButton(
            ICON_FA_ERASER,
            "Erase (E)",
            m_Mode == TileBrushMode::Erase,
            buttonSize))
        {
            m_Mode = TileBrushMode::Erase;
        }

        ImGui::SameLine(0.0f, 4.0f);

        if (EditorIconButton(
            ICON_FA_FILL_DRIP,
            "Fill (G)",
            m_Mode == TileBrushMode::Fill,
            buttonSize))
        {
            m_Mode = TileBrushMode::Fill;
        }

        ImGui::SameLine(0.0f, 4.0f);

        if (EditorIconButton(
            ICON_FA_DICE,
            "Random Brush (R)",
            m_Mode == TileBrushMode::Random,
            buttonSize))
        {
            m_Mode = TileBrushMode::Random;
        }

        ImGui::SameLine(0.0f, 4.0f);

        if (EditorIconButton(
            ICON_FA_EYE_DROPPER,
            "Eyedropper (I)",
            m_Mode == TileBrushMode::Eyedropper,
            buttonSize))
        {
            m_Mode = TileBrushMode::Eyedropper;
        }

        ImGui::SameLine(0.0f, 10.0f);

        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

        ImGui::SameLine(0.0f, 10.0f);

        // ------------------------------------------------------------
        // View
        // ------------------------------------------------------------

        if (EditorIconButton(
            ICON_FA_EXPAND,
            "Fit View",
            false,
            buttonSize))
        {
            m_TilemapCanvasNeedsFit = true;
        }

        ImGui::SameLine(0.0f, 12.0f);

        ImGui::TextDisabled(
            "MMB Pan  |  Wheel Zoom  |  Ctrl+Wheel Brush Size");
    }

    void TilemapEditorPanel::RenderMainArea()
    {
        if (!m_Asset.IsValid())
            return;

        std::shared_ptr<Tilemap> tilemap = m_Asset->GetInstance();
        if (!tilemap)
            return;

        ImVec2 avail = ImGui::GetContentRegionAvail();

        // Workspace layout:
        //  - Atlas palette is a full-width strip at the top.
        //  - All other editor sections live underneath it.
        // This makes tile selection feel like a real tileset shelf instead of a side inspector.
        const float spacing = ImGui::GetStyle().ItemSpacing.y;
        const float minPaletteHeight = 170.0f;
        const float preferredPaletteHeight = 260.0f;
        const float maxPaletteHeight = std::max(minPaletteHeight, avail.y * 0.42f);
        const float paletteHeight = std::min(preferredPaletteHeight, maxPaletteHeight);
        const float settingsHeight = std::max(1.0f, avail.y - paletteHeight - spacing);

        BeginEditorPanel("##tile_palette_panel", ImVec2(0.0f, paletteHeight));
        RenderPalette(*tilemap);
        EndEditorPanel();

        ImGui::Spacing();

        BeginEditorPanel("##tilemap_settings_panel", ImVec2(0.0f, settingsHeight));
        RenderToolSettings(*tilemap);
        EndEditorPanel();
    }

    void TilemapEditorPanel::RenderToolSettings(Tilemap& tilemap)
    {
        HandleShortcuts(tilemap);

        if (ImGui::CollapsingHeader("Tilemap", ImGuiTreeNodeFlags_DefaultOpen))
        {
            AssetHandle atlasHandle = tilemap.GetAtlas();

            if (UI::AssetRef("Sprite Atlas", atlasHandle, AssetType::SpriteAtlas))
            {
                tilemap.SetAtlas(AssetRef<SpriteAtlasAsset>(atlasHandle));
                m_PaletteCanvasNeedsFit = true;
            }

            const int chunksXCurrent = tilemap.GetChunksX();
            const int chunksYCurrent = tilemap.GetChunksY();
            const int chunkSizeCurrent = tilemap.GetChunkSize();

            if (!m_ResizeInitialized)
            {
                m_ResizeChunksX = chunksXCurrent;
                m_ResizeChunksY = chunksYCurrent;
                m_ResizeChunkSize = chunkSizeCurrent;
                m_ResizeInitialized = true;
            }

            ImGui::TextDisabled("Size");
            ImGui::SameLine(120.0f);
            ImGui::Text("%d x %d tiles", chunksXCurrent * chunkSizeCurrent, chunksYCurrent * chunkSizeCurrent);

            UI::DragInt("Chunks X", m_ResizeChunksX, 1, 512);
            UI::DragInt("Chunks Y", m_ResizeChunksY, 1, 512);
            UI::DragInt("Chunk Size", m_ResizeChunkSize, 1, 256);

            const bool differentSize =
                m_ResizeChunksX != chunksXCurrent ||
                m_ResizeChunksY != chunksYCurrent ||
                m_ResizeChunkSize != chunkSizeCurrent;

            if (differentSize)
            {
                ImGui::TextDisabled("Preview: %d x %d tiles", m_ResizeChunksX * m_ResizeChunkSize, m_ResizeChunksY * m_ResizeChunkSize);
                ImGui::TextWrapped("Resize changes the map dimensions. Existing tiles outside the new bounds may be lost.");

                if (EditorButton("Apply Resize", false, ImVec2(120.0f, 26.0f)))
                {
                    PushUndoState(tilemap);
                    tilemap.Resize(m_ResizeChunksX, m_ResizeChunksY, m_ResizeChunkSize);
                    m_TilemapCanvasNeedsFit = true;
                }

                ImGui::SameLine();
                if (EditorButton("Reset", false, ImVec2(70.0f, 26.0f)))
                {
                    m_ResizeChunksX = chunksXCurrent;
                    m_ResizeChunksY = chunksYCurrent;
                    m_ResizeChunkSize = chunkSizeCurrent;
                }
            }

            float unitSize = tilemap.GetUnitSize();
            if (UI::DragFloat("Unit Size", unitSize, 0.001f, 256.0f, 0.01f))
            {
                tilemap.SetUnitSize(unitSize);
                m_TilemapCanvasNeedsFit = true;
            }
        }

        if (ImGui::CollapsingHeader("Brush", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const char* modeNames[] = { "Paint", "Erase", "Fill", "Random", "Eyedropper" };
            int mode = static_cast<int>(m_Mode);
            if (UI::Combo("Mode", mode, modeNames, IM_ARRAYSIZE(modeNames)))
                m_Mode = static_cast<TileBrushMode>(mode);

            const char* shapeNames[] = { "Square", "Circle" };
            int shape = static_cast<int>(m_BrushShape);
            if (UI::Combo("Shape", shape, shapeNames, IM_ARRAYSIZE(shapeNames)))
                m_BrushShape = static_cast<TileBrushShape>(shape);

            UI::SliderInt("Brush Size", m_BrushSize, 1, 31);
            UI::Checkbox("Auto paint after pick", m_AutoSwitchToPaintAfterPick);

            RenderSelectedTilePreview(tilemap);
        }

        if (ImGui::CollapsingHeader("Viewport", ImGuiTreeNodeFlags_DefaultOpen))
        {
            UI::Checkbox("Grid", m_ShowGrid);
            UI::SameLine();
            UI::Checkbox("Chunks", m_ShowChunks);
            UI::Checkbox("Empty tile outlines", m_ShowEmptyTiles);
            UI::Checkbox("Brush preview", m_ShowBrushPreview);
            UI::Checkbox("Tile coordinate overlay", m_ShowTileCoordinates);

            if (EditorButton("Fit Tilemap", false, ImVec2(110.0f, 26.0f)))
                m_TilemapCanvasNeedsFit = true;
            ImGui::SameLine();
            if (EditorButton("Fit Palette", false, ImVec2(110.0f, 26.0f)))
                m_PaletteCanvasNeedsFit = true;
        }

        if (m_Mode == TileBrushMode::Random)
            RenderRandomBrush(tilemap);
    }

    void TilemapEditorPanel::RenderSelectedTilePreview(Tilemap& tilemap)
    {
        std::shared_ptr<SpriteAtlas> atlas{};
        ImTextureID textureId{};

        ImGui::TextDisabled("Selected Tile");
        ImGui::SameLine(120.0f);
        ImGui::Text("%d", m_SelectedTile);

        if (!GetAtlasInfo(tilemap, atlas, textureId) || m_SelectedTile < 0 || !atlas->Exists(m_SelectedTile))
        {
            ImGui::TextDisabled("No tile selected.");
            return;
        }

        const SpriteFrame& frame = atlas->GetSpriteFrame(m_SelectedTile);
        ImGui::Image(textureId, ImVec2(64.0f, 64.0f), UV0(frame), UV1(frame));
        ImGui::SameLine();
        if (EditorButton("Clear", false, ImVec2(70.0f, 26.0f)))
            m_SelectedTile = -1;
    }

    void TilemapEditorPanel::HandleShortcuts(Tilemap& tilemap)
    {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantTextInput)
            return;

        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z))
        {
            Undo(tilemap);
            return;
        }

        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y))
        {
            Redo(tilemap);
            return;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_P)) m_Mode = TileBrushMode::Paint;
        if (ImGui::IsKeyPressed(ImGuiKey_E)) m_Mode = TileBrushMode::Erase;
        if (ImGui::IsKeyPressed(ImGuiKey_G)) m_Mode = TileBrushMode::Fill;
        if (ImGui::IsKeyPressed(ImGuiKey_R)) m_Mode = TileBrushMode::Random;
        if (ImGui::IsKeyPressed(ImGuiKey_I)) m_Mode = TileBrushMode::Eyedropper;
    }

    void TilemapEditorPanel::FitTilemapCanvas(Tilemap& tilemap)
    {
        const float unitSize = tilemap.GetUnitSize();
        const glm::vec2 mapSize{
            static_cast<float>(tilemap.GetChunksX() * tilemap.GetChunkSize()) * unitSize,
            static_cast<float>(tilemap.GetChunksY() * tilemap.GetChunkSize()) * unitSize
        };

        m_TilemapCanvas.FitContent({ 0.0f, 0.0f }, mapSize, 36.0f);
        m_TilemapCanvasNeedsFit = false;
    }

    void TilemapEditorPanel::FitPaletteCanvas(const glm::vec2& textureSize)
    {
        m_PaletteCanvas.FitContent({ 0.0f, 0.0f }, textureSize, 24.0f);
        m_PaletteCanvasNeedsFit = false;
    }

    void TilemapEditorPanel::RenderPalette(Tilemap& tilemap)
    {
        SectionHeader("Tile Palette");

        std::shared_ptr<SpriteAtlas> atlas{};
        ImTextureID textureId{};

        if (!GetAtlasInfo(tilemap, atlas, textureId))
        {
            ImGui::TextDisabled("Assign a valid sprite atlas to use the palette.");
            return;
        }

        auto textureAsset = atlas->GetTexture().Get();
        if (!textureAsset)
            return;

        auto texture = textureAsset->GetInstance();
        if (!texture)
            return;

        glm::vec2 textureSize{
            static_cast<float>(texture->GetWidth()),
            static_cast<float>(texture->GetHeight())
        };

        ImGui::TextDisabled("Click a sprite to select it. MMB pan, wheel zoom.");
        ImGui::SameLine();
        if (EditorButton("Fit", false, ImVec2(52.0f, 24.0f)))
            m_PaletteCanvasNeedsFit = true;
        ImGui::SameLine();
        if (EditorButton("Clear", false, ImVec2(64.0f, 24.0f)))
            m_SelectedTile = -1;
        ImGui::Spacing();

        ImVec2 canvasSize = ImGui::GetContentRegionAvail();

        // Leave room for bottom text/status if needed.
        canvasSize.y = std::max(120.0f, canvasSize.y - 4.0f);

        BeginEditorPanel("##palette_atlas_canvas_region", canvasSize);

        m_PaletteCanvas.Begin("TilePaletteCanvas");

        if (m_PaletteCanvasNeedsFit)
            FitPaletteCanvas(textureSize);

        m_PaletteCanvas.DrawImage(
            textureId,
            { 0.0f, 0.0f },
            textureSize,
            { 0.0f, 1.0f },
            { 1.0f, 0.0f }
        );

        m_PaletteCanvas.DrawRect(
            { 0.0f, 0.0f },
            textureSize,
            Border(),
            1.0f
        );

        glm::vec2 mouse = m_PaletteCanvas.GetMouseCanvas();

        int hoveredFrame = HitTestAtlasFrame(*atlas, mouse, textureSize);

        if (m_PaletteCanvas.IsHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            m_SelectedTile = hoveredFrame;
            if (hoveredFrame >= 0 && m_Mode == TileBrushMode::Eyedropper)
                m_Mode = TileBrushMode::Paint;
        }

        for (int frameId : atlas->GetAllFrameIDs())
        {
            if (!atlas->Exists(frameId))
                continue;

            const SpriteFrame& frame = atlas->GetSpriteFrame(frameId);

            glm::vec2 pixelPos = UVToPixel(frame, textureSize);
            glm::vec2 pixelSize = SizeToPixel(frame, textureSize);

            ImU32 color = IM_COL32(255, 255, 255, 70);
            float thickness = 1.0f;

            if (frameId == m_SelectedTile)
            {
                color = Accent(1.0f);
                thickness = 2.5f;
            }
            else if (frameId == hoveredFrame)
            {
                color = Accent(0.75f);
                thickness = 2.0f;
            }

            m_PaletteCanvas.DrawRect(
                pixelPos,
                pixelSize,
                color,
                thickness
            );
        }

        if (m_PaletteCanvas.IsHovered() && hoveredFrame >= 0)
        {
            ImGui::BeginTooltip();
            ImGui::Text("Tile %d", hoveredFrame);
            ImGui::EndTooltip();
        }

        m_PaletteCanvas.End();

        EndEditorPanel();
    }

    void TilemapEditorPanel::RenderRandomBrush(Tilemap& tilemap)
    {
        if (!ImGui::CollapsingHeader("Random Brush", ImGuiTreeNodeFlags_DefaultOpen))
            return;

        std::shared_ptr<SpriteAtlas> atlas{};
        ImTextureID textureId{};

        if (!GetAtlasInfo(tilemap, atlas, textureId))
            return;

        if (EditorButton("Add Selected", false, ImVec2(110.0f, 26.0f)))
        {
            if (m_SelectedTile >= 0 && atlas->Exists(m_SelectedTile) &&
                std::find(m_RandomBrushTiles.begin(), m_RandomBrushTiles.end(), m_SelectedTile) == m_RandomBrushTiles.end())
            {
                m_RandomBrushTiles.push_back(m_SelectedTile);
                m_RandomBrushWeights.push_back(1.0f);
            }
        }

        ImGui::SameLine();
        if (EditorButton("Clear List", false, ImVec2(90.0f, 26.0f)))
        {
            m_RandomBrushTiles.clear();
            m_RandomBrushWeights.clear();
        }

        ImGui::TextDisabled("Click atlas tiles below to toggle them in the random brush.");

        const float cell = 38.0f;
        const int columns = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / cell));
        int column = 0;

        for (int frameId : atlas->GetAllFrameIDs())
        {
            if (!atlas->Exists(frameId))
                continue;

            const SpriteFrame& frame = atlas->GetSpriteFrame(frameId);

            auto it = std::find(m_RandomBrushTiles.begin(), m_RandomBrushTiles.end(), frameId);
            const bool selected = it != m_RandomBrushTiles.end();

            ImGui::PushID(frameId);
            ImGui::PushStyleColor(ImGuiCol_Button, selected ? Accent(0.28f) : Surface());
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, selected ? Accent(0.38f) : SurfaceHover());

            if (ImGui::ImageButton("##random_tile", textureId, ImVec2(30.0f, 30.0f), UV0(frame), UV1(frame)))
            {
                if (!selected)
                {
                    m_RandomBrushTiles.push_back(frameId);
                    m_RandomBrushWeights.push_back(1.0f);
                }
                else
                {
                    const int idx = static_cast<int>(std::distance(m_RandomBrushTiles.begin(), it));
                    m_RandomBrushTiles.erase(m_RandomBrushTiles.begin() + idx);
                    m_RandomBrushWeights.erase(m_RandomBrushWeights.begin() + idx);
                }
            }

            ImGui::PopStyleColor(2);
            ImGui::PopID();

            ++column;
            if (column < columns)
                ImGui::SameLine();
            else
                column = 0;
        }

        if (!m_RandomBrushTiles.empty())
        {
            SectionHeader("Weights");
            for (size_t i = 0; i < m_RandomBrushTiles.size(); ++i)
            {
                ImGui::PushID(static_cast<int>(i));
                ImGui::Text("Tile %d", m_RandomBrushTiles[i]);
                ImGui::SameLine(90.0f);
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::SliderFloat("##weight", &m_RandomBrushWeights[i], 0.0f, 5.0f, "%.2f");
                ImGui::PopID();
            }
        }
    }

    void TilemapEditorPanel::RenderViewportTilemap(Tilemap& tilemap)
    {
        std::shared_ptr<SpriteAtlas> atlas{};
        ImTextureID textureId{};

        if (!GetAtlasInfo(tilemap, atlas, textureId))
        {
            ImGui::TextDisabled("Assign a valid sprite atlas in the Tilemap Editor.");
            return;
        }

        HandleShortcuts(tilemap);

        m_TilemapCanvas.Begin("TilemapViewportCanvas");

        if (m_TilemapCanvasNeedsFit)
            FitTilemapCanvas(tilemap);

        RenderTiles(tilemap);

        if (m_ShowGrid)
            RenderTilemapGrid(tilemap);

        if (m_ShowChunks)
            RenderChunkGrid(tilemap);

        glm::vec2 mouse = m_TilemapCanvas.GetMouseCanvas();

        const float unitSize = tilemap.GetUnitSize();

        glm::ivec2 tile{
            static_cast<int>(std::floor(mouse.x / unitSize)),
            static_cast<int>(std::floor(mouse.y / unitSize))
        };

        m_HoveredTile = tilemap.IsValidTile(tile.x, tile.y) ? tile : glm::ivec2{ -1, -1 };

        if (tilemap.IsValidTile(tile.x, tile.y))
        {
            if (m_ShowBrushPreview)
                RenderBrushPreview(tilemap, tile);
            HandlePainting(tilemap, tile);
            RenderViewportOverlay(tilemap, tile);
        }
        else
        {
            m_EditInProgress = false;
        }

        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
            m_EditInProgress = false;

        m_TilemapCanvas.End();
    }

    void TilemapEditorPanel::RenderViewportOverlay(Tilemap& tilemap, const glm::ivec2& tile)
    {
        if (!m_ShowTileCoordinates)
            return;

        const int currentTile = tilemap.GetTile(tile.x, tile.y);
        ImGui::SetNextWindowBgAlpha(0.82f);
        ImVec2 mousePos = ImGui::GetMousePos();
        ImGui::SetNextWindowPos(
            ImVec2(mousePos.x + 16.0f, mousePos.y + 16.0f),
            ImGuiCond_Always
        );

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav;

        if (ImGui::Begin("##tilemap_hover_overlay", nullptr, flags))
        {
            ImGui::Text("Tile: %d, %d", tile.x, tile.y);
            ImGui::TextDisabled("Value: %d", currentTile);
        }
        ImGui::End();
    }

    void TilemapEditorPanel::RenderTiles(Tilemap& tilemap)
    {
        std::shared_ptr<SpriteAtlas> atlas{};
        ImTextureID textureId{};

        if (!GetAtlasInfo(tilemap, atlas, textureId))
            return;

        const float unitSize = tilemap.GetUnitSize();

        const int width = tilemap.GetChunksX() * tilemap.GetChunkSize();
        const int height = tilemap.GetChunksY() * tilemap.GetChunkSize();

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int tileId = tilemap.GetTile(x, y);

                if (tileId < 0)
                {
                    if (m_ShowEmptyTiles)
                    {
                        m_TilemapCanvas.DrawRect(
                            { x * unitSize, y * unitSize },
                            { unitSize, unitSize },
                            IM_COL32(255, 255, 255, 20),
                            1.0f
                        );
                    }

                    continue;
                }

                if (!atlas->Exists(tileId))
                    continue;

                const SpriteFrame& frame = atlas->GetSpriteFrame(tileId);

                m_TilemapCanvas.DrawImage(
                    textureId,
                    { x * unitSize, y * unitSize },
                    { unitSize, unitSize },
                    { frame.UV.x, frame.UV.y + frame.Size.y },
                    { frame.UV.x + frame.Size.x, frame.UV.y }
                );
            }
        }
    }

    void TilemapEditorPanel::RenderTilemapGrid(Tilemap& tilemap)
    {
        const float unitSize = tilemap.GetUnitSize();

        m_TilemapCanvas.DrawGrid(
            unitSize,
            IM_COL32(255, 255, 255, 35)
        );
    }

    void TilemapEditorPanel::RenderChunkGrid(Tilemap& tilemap)
    {
        const float unitSize = tilemap.GetUnitSize();
        const float chunkWorldSize = static_cast<float>(tilemap.GetChunkSize()) * unitSize;

        m_TilemapCanvas.DrawGrid(
            chunkWorldSize,
            Accent(0.35f)
        );
    }

    void TilemapEditorPanel::RenderBrushPreview(Tilemap& tilemap, const glm::ivec2& tile)
    {
        const float unitSize = tilemap.GetUnitSize();

        int half = m_BrushSize / 2;

        float radius = static_cast<float>(m_BrushSize) * 0.5f;
        float radiusSq = radius * radius;

        for (int y = -half; y <= half; ++y)
        {
            for (int x = -half; x <= half; ++x)
            {
                glm::ivec2 p = tile + glm::ivec2{ x, y };

                if (!tilemap.IsValidTile(p.x, p.y))
                    continue;

                if (m_BrushShape == TileBrushShape::Circle)
                {
                    float dx = static_cast<float>(x);
                    float dy = static_cast<float>(y);

                    if ((dx * dx + dy * dy) > radiusSq)
                        continue;
                }

                const bool erase = m_Mode == TileBrushMode::Erase;

                m_TilemapCanvas.DrawFilledRect(
                    { p.x * unitSize, p.y * unitSize },
                    { unitSize, unitSize },
                    erase ? IM_COL32(255, 70, 70, 45) : Accent(0.18f)
                );

                m_TilemapCanvas.DrawRect(
                    { p.x * unitSize, p.y * unitSize },
                    { unitSize, unitSize },
                    erase ? IM_COL32(255, 90, 90, 190) : Accent(0.9f),
                    2.0f
                );
            }
        }
    }

    void TilemapEditorPanel::HandlePainting(Tilemap& tilemap, const glm::ivec2& tile)
    {
        if (!m_TilemapCanvas.IsHovered())
            return;

        ImGuiIO& io = ImGui::GetIO();

        if (io.KeyCtrl && io.MouseWheel != 0.0f)
        {
            m_BrushSize += io.MouseWheel > 0.0f ? 1 : -1;
            m_BrushSize = std::clamp(m_BrushSize, 1, 31);
            return;
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            PushUndoState(tilemap);
            m_EditInProgress = true;

            if (m_Mode == TileBrushMode::Eyedropper)
            {
                m_SelectedTile = tilemap.GetTile(tile.x, tile.y);
                if (m_AutoSwitchToPaintAfterPick)
                    m_Mode = TileBrushMode::Paint;
                return;
            }

            if (m_Mode == TileBrushMode::Fill)
            {
                FloodFill(tilemap, tile.x, tile.y, m_SelectedTile);
                return;
            }
        }

        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            m_EditInProgress = false;
            return;
        }

        if (!m_EditInProgress)
        {
            PushUndoState(tilemap);
            m_EditInProgress = true;
        }

        switch (m_Mode)
        {
        case TileBrushMode::Paint:
            PaintBrush(tilemap, tile, m_SelectedTile);
            break;

        case TileBrushMode::Erase:
            PaintBrush(tilemap, tile, -1);
            break;

        case TileBrushMode::Random:
            PaintBrush(tilemap, tile, SelectRandomTile());
            break;

        case TileBrushMode::Fill:
        case TileBrushMode::Eyedropper:
            break;
        }
    }

    void TilemapEditorPanel::PaintBrush(Tilemap& tilemap, const glm::ivec2& tile, int tileId)
    {
        int half = m_BrushSize / 2;

        float radius = static_cast<float>(m_BrushSize) * 0.5f;
        float radiusSq = radius * radius;

        for (int y = -half; y <= half; ++y)
        {
            for (int x = -half; x <= half; ++x)
            {
                glm::ivec2 p = tile + glm::ivec2{ x, y };

                if (!tilemap.IsValidTile(p.x, p.y))
                    continue;

                if (m_BrushShape == TileBrushShape::Circle)
                {
                    float dx = static_cast<float>(x);
                    float dy = static_cast<float>(y);

                    if ((dx * dx + dy * dy) > radiusSq)
                        continue;
                }

                tilemap.SetTile(p.x, p.y, tileId);
            }
        }
    }

    void TilemapEditorPanel::FloodFill(Tilemap& tilemap, int x, int y, int newTile)
    {
        if (!tilemap.IsValidTile(x, y))
            return;

        const int target = tilemap.GetTile(x, y);

        if (target == newTile)
            return;

        std::queue<glm::ivec2> q;
        std::set<std::pair<int, int>> visited;

        q.push({ x, y });

        while (!q.empty())
        {
            glm::ivec2 p = q.front();
            q.pop();

            std::pair<int, int> key{ p.x, p.y };

            if (visited.find(key) != visited.end())
                continue;

            visited.insert(key);

            if (!tilemap.IsValidTile(p.x, p.y))
                continue;

            if (tilemap.GetTile(p.x, p.y) != target)
                continue;

            tilemap.SetTile(p.x, p.y, newTile);

            q.push({ p.x + 1, p.y });
            q.push({ p.x - 1, p.y });
            q.push({ p.x, p.y + 1 });
            q.push({ p.x, p.y - 1 });
        }
    }

    int TilemapEditorPanel::SelectRandomTile() const
    {
        if (m_RandomBrushTiles.empty())
            return m_SelectedTile;

        float total = 0.0f;

        for (float weight : m_RandomBrushWeights)
            total += std::max(0.0f, weight);

        if (total <= 0.0f)
            return m_RandomBrushTiles.front();

        float r =
            (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * total;

        for (size_t i = 0; i < m_RandomBrushTiles.size(); ++i)
        {
            float weight = std::max(0.0f, m_RandomBrushWeights[i]);

            if (r <= weight)
                return m_RandomBrushTiles[i];

            r -= weight;
        }

        return m_RandomBrushTiles.back();
    }

    TilemapSnapshot TilemapEditorPanel::CaptureSnapshot(Tilemap& tilemap) const
    {
        TilemapSnapshot snapshot;
        snapshot.ChunksX = tilemap.GetChunksX();
        snapshot.ChunksY = tilemap.GetChunksY();
        snapshot.ChunkSize = tilemap.GetChunkSize();
        snapshot.UnitSize = tilemap.GetUnitSize();

        const int width = snapshot.ChunksX * snapshot.ChunkSize;
        const int height = snapshot.ChunksY * snapshot.ChunkSize;
        snapshot.Tiles.reserve(static_cast<size_t>(width * height));

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
                snapshot.Tiles.push_back(tilemap.GetTile(x, y));
        }

        return snapshot;
    }

    void TilemapEditorPanel::RestoreSnapshot(Tilemap& tilemap, const TilemapSnapshot& snapshot)
    {
        if (snapshot.ChunksX <= 0 || snapshot.ChunksY <= 0 || snapshot.ChunkSize <= 0)
            return;

        tilemap.Resize(snapshot.ChunksX, snapshot.ChunksY, snapshot.ChunkSize);
        tilemap.SetUnitSize(snapshot.UnitSize);

        const int width = snapshot.ChunksX * snapshot.ChunkSize;
        const int height = snapshot.ChunksY * snapshot.ChunkSize;
        const size_t expected = static_cast<size_t>(width * height);

        if (snapshot.Tiles.size() < expected)
            return;

        size_t index = 0;
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
                tilemap.SetTile(x, y, snapshot.Tiles[index++]);
        }

        m_TilemapCanvasNeedsFit = true;
    }

    void TilemapEditorPanel::PushUndoState(Tilemap& tilemap)
    {
        m_UndoStack.push_back(CaptureSnapshot(tilemap));
        if (static_cast<int>(m_UndoStack.size()) > m_MaxUndoSteps)
            m_UndoStack.erase(m_UndoStack.begin());

        m_RedoStack.clear();
    }

    void TilemapEditorPanel::Undo(Tilemap& tilemap)
    {
        return; //skip for now

        if (m_UndoStack.empty())
            return;

        m_RedoStack.push_back(CaptureSnapshot(tilemap));

        TilemapSnapshot snapshot = m_UndoStack.back();
        m_UndoStack.pop_back();
        RestoreSnapshot(tilemap, snapshot);
    }

    void TilemapEditorPanel::Redo(Tilemap& tilemap)
    {
        return; //skip for now

        if (m_RedoStack.empty())
            return;

        m_UndoStack.push_back(CaptureSnapshot(tilemap));

        TilemapSnapshot snapshot = m_RedoStack.back();
        m_RedoStack.pop_back();
        RestoreSnapshot(tilemap, snapshot);
    }

    bool TilemapEditorPanel::GetAtlasInfo(
        Tilemap& tilemap,
        std::shared_ptr<SpriteAtlas>& outAtlas,
        ImTextureID& outTextureId) const
    {
        if (!tilemap.GetAtlas().IsValid())
            return false;

        auto atlasAsset = tilemap.GetAtlas().Get();

        if (!atlasAsset)
            return false;

        outAtlas = atlasAsset->GetInstance();

        if (!outAtlas || !outAtlas->GetTexture().IsValid())
            return false;

        auto textureAsset = outAtlas->GetTexture().Get();

        if (!textureAsset)
            return false;

        auto texture = textureAsset->GetInstance();

        if (!texture)
            return false;

        outTextureId = texture->GetRendererID();
        return true;
    }

    ImVec2 TilemapEditorPanel::UV0(const SpriteFrame& frame)
    {
        return ImVec2(frame.UV.x, frame.UV.y + frame.Size.y);
    }

    ImVec2 TilemapEditorPanel::UV1(const SpriteFrame& frame)
    {
        return ImVec2(frame.UV.x + frame.Size.x, frame.UV.y);
    }
    void TilemapEditorPanel::RenderAtlasPalette(Tilemap& tilemap)
    {
        std::shared_ptr<SpriteAtlas> atlas{};
        ImTextureID textureId{};

        if (!GetAtlasInfo(tilemap, atlas, textureId))
        {
            ImGui::TextDisabled("Assign a valid sprite atlas to use the palette.");
            return;
        }

        auto textureAsset = atlas->GetTexture().Get();
        if (!textureAsset)
            return;

        auto texture = textureAsset->GetInstance();
        if (!texture)
            return;

        glm::vec2 textureSize{
            static_cast<float>(texture->GetWidth()),
            static_cast<float>(texture->GetHeight())
        };

        ImVec2 avail = ImGui::GetContentRegionAvail();

        float scale = std::min(
            avail.x / textureSize.x,
            avail.y / textureSize.y
        );

        scale = std::clamp(scale, 0.1f, 4.0f);

        ImVec2 imageSize{
            textureSize.x * scale,
            textureSize.y * scale
        };

        ImVec2 imageMin = ImGui::GetCursorScreenPos();

        ImGui::Image(
            textureId,
            imageSize,
            ImVec2(0.0f, 1.0f),
            ImVec2(1.0f, 0.0f)
        );

        ImVec2 imageMax{
            imageMin.x + imageSize.x,
            imageMin.y + imageSize.y
        };

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        drawList->AddRect(
            imageMin,
            imageMax,
            Border(),
            ImGui::GetStyle().FrameRounding
        );

        ImVec2 mouse = ImGui::GetMousePos();

        bool inside =
            mouse.x >= imageMin.x &&
            mouse.y >= imageMin.y &&
            mouse.x <= imageMax.x &&
            mouse.y <= imageMax.y;

        glm::vec2 atlasPixel{
            (mouse.x - imageMin.x) / scale,
            (mouse.y - imageMin.y) / scale
        };

        int hoveredFrame = inside
            ? HitTestAtlasFrame(*atlas, atlasPixel, textureSize)
            : -1;

        if (inside && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            m_SelectedTile = hoveredFrame;

        for (int frameId : atlas->GetAllFrameIDs())
        {
            if (!atlas->Exists(frameId))
                continue;

            const SpriteFrame& frame = atlas->GetSpriteFrame(frameId);

            glm::vec2 pixelPos = UVToPixel(frame, textureSize);
            glm::vec2 pixelSize = SizeToPixel(frame, textureSize);

            ImVec2 min{
                imageMin.x + pixelPos.x * scale,
                imageMin.y + pixelPos.y * scale
            };

            ImVec2 max{
                min.x + pixelSize.x * scale,
                min.y + pixelSize.y * scale
            };

            ImU32 color = IM_COL32(255, 255, 255, 80);
            float thickness = 1.0f;

            if (frameId == m_SelectedTile)
            {
                color = Accent(1.0f);
                thickness = 2.5f;
            }
            else if (frameId == hoveredFrame)
            {
                color = Accent(0.75f);
                thickness = 2.0f;
            }

            drawList->AddRect(
                min,
                max,
                color,
                ImGui::GetStyle().FrameRounding,
                0,
                thickness
            );
        }

        ImGui::Spacing();

        if (m_SelectedTile >= 0)
            ImGui::TextDisabled("Selected tile: %d", m_SelectedTile);
        else
            ImGui::TextDisabled("Click a sprite in the atlas to select a tile.");
    }

    int TilemapEditorPanel::HitTestAtlasFrame(
        SpriteAtlas& atlas,
        const glm::vec2& pixelPos,
        const glm::vec2& textureSize) const
    {
        std::vector<int> ids = atlas.GetAllFrameIDs();

        for (auto it = ids.rbegin(); it != ids.rend(); ++it)
        {
            int id = *it;

            if (!atlas.Exists(id))
                continue;

            const SpriteFrame& frame = atlas.GetSpriteFrame(id);

            glm::vec2 pos = UVToPixel(frame, textureSize);
            glm::vec2 size = SizeToPixel(frame, textureSize);

            if (pixelPos.x >= pos.x &&
                pixelPos.y >= pos.y &&
                pixelPos.x <= pos.x + size.x &&
                pixelPos.y <= pos.y + size.y)
            {
                return id;
            }
        }

        return -1;
    }

    glm::vec2 TilemapEditorPanel::UVToPixel(
        const SpriteFrame& frame,
        const glm::vec2& textureSize)
    {
        return {
            frame.UV.x * textureSize.x,
            (1.0f - frame.UV.y - frame.Size.y) * textureSize.y
        };
    }

    glm::vec2 TilemapEditorPanel::SizeToPixel(
        const SpriteFrame& frame,
        const glm::vec2& textureSize)
    {
        return {
            frame.Size.x * textureSize.x,
            frame.Size.y * textureSize.y
        };
    }
}