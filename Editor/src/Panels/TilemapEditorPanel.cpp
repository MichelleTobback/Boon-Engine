#include "Panels/TilemapEditorPanel.h"

#include <imgui.h>

#include "Assets/AssetDatabase.h"
#include "UI/UI.h"

#include <Renderer/Texture.h>

#include <algorithm>
#include <cstdlib>
#include <cmath>

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
}

namespace BoonEditor
{
    TilemapEditorPanel::TilemapEditorPanel(const std::string& name, EditorContext* pContext)
        : AssetEditor<TilemapAsset>(name, pContext)
    {
        m_TilemapCanvas.SetZoomLimits(0.f, 128.f);
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

        ImGui::Text("Tilemap Editor");
        ImGui::SameLine();
        ImGui::TextDisabled("Paint tiles, configure brushes, and edit map data");

        ImGui::Spacing();

        if (EditorButton("Save##tilemap_save", false, ImVec2(72.0f, 28.0f)))
            AssetDatabase::Get().Export<TilemapAsset>(m_Asset);

        ImGui::SameLine(0.0f, 16.0f);

        if (EditorButton("Paint##mode_paint", m_Mode == TileBrushMode::Paint))
            m_Mode = TileBrushMode::Paint;

        ImGui::SameLine();

        if (EditorButton("Erase##mode_erase", m_Mode == TileBrushMode::Erase))
            m_Mode = TileBrushMode::Erase;

        ImGui::SameLine();

        if (EditorButton("Fill##mode_fill", m_Mode == TileBrushMode::Fill))
            m_Mode = TileBrushMode::Fill;

        ImGui::SameLine();

        if (EditorButton("Random##mode_random", m_Mode == TileBrushMode::Random))
            m_Mode = TileBrushMode::Random;

        ImGui::SameLine();

        if (EditorButton("Pick##mode_pick", m_Mode == TileBrushMode::Eyedropper))
            m_Mode = TileBrushMode::Eyedropper;

        ImGui::SameLine(0.0f, 16.0f);
        ImGui::TextDisabled("MMB drag to pan | Wheel to zoom | Left click/drag to edit");
    }

    void TilemapEditorPanel::RenderMainArea()
    {
        if (!m_Asset.IsValid())
            return;

        std::shared_ptr<Tilemap> tilemap = m_Asset->GetInstance();
        if (!tilemap)
            return;

        ImVec2 avail = ImGui::GetContentRegionAvail();

        const float settingsWidth = 340.0f;

        ImGui::BeginChild("##tilemap_left", ImVec2(avail.x - settingsWidth - 10.0f, avail.y), false);

        BeginEditorPanel("##tile_palette_panel", ImVec2(0.0f, avail.y));
        RenderPalette(*tilemap);
        EndEditorPanel();

        ImGui::EndChild();

        ImGui::SameLine();

        BeginEditorPanel("##tilemap_settings_panel", ImVec2(settingsWidth, 0.0f));
        RenderToolSettings(*tilemap);
        EndEditorPanel();
    }

    void TilemapEditorPanel::RenderToolSettings(Tilemap& tilemap)
    {
        SectionHeader("Tilemap");

        AssetHandle atlasHandle = tilemap.GetAtlas();

        if (UI::AssetRef("Sprite Atlas", atlasHandle, AssetType::SpriteAtlas))
            tilemap.SetAtlas(AssetRef<SpriteAtlasAsset>(atlasHandle));

        int chunksX = tilemap.GetChunksX();
        int chunksY = tilemap.GetChunksY();
        int chunkSize = tilemap.GetChunkSize();

        ImGui::TextDisabled("Map Size");
        ImGui::Text("%d x %d tiles", chunksX * chunkSize, chunksY * chunkSize);

        bool resizeChanged = false;
        resizeChanged |= ImGui::DragInt("Chunks X", &chunksX, 1.0f, 1, 512);
        resizeChanged |= ImGui::DragInt("Chunks Y", &chunksY, 1.0f, 1, 512);
        resizeChanged |= ImGui::DragInt("Chunk Size", &chunkSize, 1.0f, 1, 256);

        if (resizeChanged && ImGui::Button("Apply Resize"))
            tilemap.Resize(chunksX, chunksY, chunkSize);

        float unitSize = tilemap.GetUnitSize();
        if (ImGui::DragFloat("Unit Size", &unitSize, 0.01f, 0.001f, 256.0f))
            tilemap.SetUnitSize(unitSize);

        ImGui::Checkbox("Show Grid", &m_ShowGrid);
        ImGui::Checkbox("Show Chunks", &m_ShowChunks);
        ImGui::Checkbox("Show Empty Tiles", &m_ShowEmptyTiles);

        SectionHeader("Brush");

        const char* modeNames[] =
        {
            "Paint",
            "Erase",
            "Fill",
            "Random",
            "Eyedropper"
        };

        int mode = static_cast<int>(m_Mode);

        if (ImGui::Combo("Mode", &mode, modeNames, IM_ARRAYSIZE(modeNames)))
            m_Mode = static_cast<TileBrushMode>(mode);

        const char* shapeNames[] =
        {
            "Square",
            "Circle"
        };

        int shape = static_cast<int>(m_BrushShape);

        if (ImGui::Combo("Shape", &shape, shapeNames, IM_ARRAYSIZE(shapeNames)))
            m_BrushShape = static_cast<TileBrushShape>(shape);

        ImGui::SliderInt("Brush Size", &m_BrushSize, 1, 15);

        ImGui::TextDisabled("Selected Tile");
        ImGui::SameLine(120.0f);
        ImGui::Text("%d", m_SelectedTile);

        if (m_Mode == TileBrushMode::Random)
            RenderRandomBrush(tilemap);
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

        ImGui::TextDisabled("Selected Tile");
        ImGui::SameLine(120.0f);
        ImGui::Text("%d", m_SelectedTile);

        ImGui::SameLine();

        if (EditorButton("Clear##clear_selected_tile", false, ImVec2(64.0f, 24.0f)))
            m_SelectedTile = -1;

        ImGui::TextDisabled("Wheel zoom | MMB drag pan | Click sprite to select");
        ImGui::Spacing();

        ImVec2 canvasSize = ImGui::GetContentRegionAvail();

        // Leave room for bottom text/status if needed.
        canvasSize.y = std::max(120.0f, canvasSize.y - 4.0f);

        BeginEditorPanel("##palette_atlas_canvas_region", canvasSize);

        m_PaletteCanvas.Begin("TilePaletteCanvas");

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

        if (m_PaletteCanvas.IsHovered() &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            m_SelectedTile = hoveredFrame;
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

        m_PaletteCanvas.End();

        EndEditorPanel();
    }

    void TilemapEditorPanel::RenderRandomBrush(Tilemap& tilemap)
    {
        SectionHeader("Random Brush");

        std::shared_ptr<SpriteAtlas> atlas{};
        ImTextureID textureId{};

        if (!GetAtlasInfo(tilemap, atlas, textureId))
            return;

        for (int frameId : atlas->GetAllFrameIDs())
        {
            if (!atlas->Exists(frameId))
                continue;

            const SpriteFrame& frame = atlas->GetSpriteFrame(frameId);

            auto it = std::find(
                m_RandomBrushTiles.begin(),
                m_RandomBrushTiles.end(),
                frameId
            );

            const bool selected = it != m_RandomBrushTiles.end();

            ImGui::PushID(frameId);

            ImGui::PushStyleColor(ImGuiCol_Button, selected ? Accent(0.25f) : Surface());

            if (ImGui::ImageButton(
                "##random_tile",
                textureId,
                ImVec2(32.0f, 32.0f),
                UV0(frame),
                UV1(frame)))
            {
                if (!selected)
                {
                    m_RandomBrushTiles.push_back(frameId);
                    m_RandomBrushWeights.push_back(1.0f);
                }
                else
                {
                    int idx = static_cast<int>(std::distance(m_RandomBrushTiles.begin(), it));

                    m_RandomBrushTiles.erase(m_RandomBrushTiles.begin() + idx);
                    m_RandomBrushWeights.erase(m_RandomBrushWeights.begin() + idx);
                }
            }

            ImGui::PopStyleColor();

            if (selected)
            {
                int idx = static_cast<int>(std::distance(m_RandomBrushTiles.begin(), it));

                ImGui::SameLine();
                ImGui::SetNextItemWidth(120.0f);
                ImGui::SliderFloat("Weight", &m_RandomBrushWeights[idx], 0.0f, 5.0f);
            }

            ImGui::PopID();
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

        m_TilemapCanvas.Begin("TilemapViewportCanvas");

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

        if (tilemap.IsValidTile(tile.x, tile.y))
        {
            RenderBrushPreview(tilemap, tile);
            HandlePainting(tilemap, tile);
        }

        m_TilemapCanvas.End();
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

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            if (m_Mode == TileBrushMode::Eyedropper)
            {
                m_SelectedTile = tilemap.GetTile(tile.x, tile.y);
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
            return;

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