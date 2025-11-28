#include "Panels/TilemapEditorPanel.h"

#include <imgui.h>
#include <Core/ServiceLocator.h>
#include <Input/Input.h>
#include <Renderer/Renderer2D.h>
#include <Renderer/Texture.h>
#include <Scene/SceneManager.h>
#include <Component/TilemapRendererComponent.h>
#include "Panels/ViewportPanel.h"
#include <BoonDebug/DebugRenderer.h>

#include "Assets/AssetDatabase.h"

#include "UI/UI.h"

using namespace BoonEditor;
using namespace Boon;

TilemapEditorPanel::TilemapEditorPanel(const std::string& name, DragDropRouter* pRouter)
    : AssetEditor<TilemapAsset>(name, pRouter) {}

void TilemapEditorPanel::Update() {}

void TilemapEditorPanel::BuildPreviewScene(Scene& scene)
{
    scene.Instantiate().AddComponent<TilemapRendererComponent>().tilemap = m_Asset;
}

//
// ──────────────────────────────────────────
//   TOOLBAR
// ──────────────────────────────────────────
//
void TilemapEditorPanel::RenderToolbar()
{
    auto tilemap = m_Asset->GetInstance();
    if (ImGui::Button("save"))
    {
        AssetDatabase::Get().Export<TilemapAsset>(m_Asset);
    }

    ImGui::SameLine(0.f, 25.f);

    // Brush action modes
    if (ImGui::Button("Paint"))  m_Mode = TileBrushMode::Paint;
    ImGui::SameLine();
    if (ImGui::Button("Erase"))  m_Mode = TileBrushMode::Erase;
    ImGui::SameLine();
    if (ImGui::Button("Fill"))   m_Mode = TileBrushMode::FloodFill;
    ImGui::SameLine();
    if (ImGui::Button("Stamp"))  m_Mode = TileBrushMode::Stamp;
    ImGui::SameLine();
    if (ImGui::Button("Random")) m_Mode = TileBrushMode::Random;

    ImGui::Separator();

    auto atlasAsset = m_Asset->GetInstance()->GetAtlas();
    AssetHandle handle = atlasAsset;
    if (UI::AssetRef("sprite atlas", handle, AssetType::SpriteAtlas))
    {
        m_Asset->GetInstance()->SetAtlas(AssetRef<SpriteAtlasAsset>(handle));
    }

    // Brush shape
    const char* shapes[] = { "Square", "Circle" };
    int shape = (int)m_BrushShape;
    if (UI::Combo("brush shape", shape, shapes, 2))
    {
        m_BrushShape = (TileBrushShape)shape;
    }

    UI::SliderInt("size", m_BrushSize, 1, 15);

    glm::ivec2 chunks{  };
    if (UI::DragInt2("chunks", chunks))
    {

    }
    int chunkSize = tilemap->GetChunkSize();
    if (UI::DragInt("chunk size", chunkSize))
    {
        
    }
    float unitSize = tilemap->GetUnitSize();
    if (UI::DragFloat("unit size", unitSize))
    {
        tilemap->SetUnitSize(unitSize);
    }

    if (m_Mode == TileBrushMode::Random)
    {
        auto atlas = m_Asset->GetInstance()->GetAtlas().Instance();
        auto tex = atlas->GetTexture().Instance();

        ImGui::Text("Random brush tiles:");
        for (int i : atlas->GetAllFrameIDs())
        {
            auto& frame = atlas->GetSpriteFrame(i);
            ImGui::PushID(i);
            bool selected =
                std::find(m_RandomBrushTiles.begin(), m_RandomBrushTiles.end(), i)
                != m_RandomBrushTiles.end();

            if (ImGui::ImageButton("##tile", (ImTextureID)tex->GetRendererID(),
                ImVec2(32, 32),
                ImVec2(frame.UV.x, frame.UV.y + frame.Size.y),
                ImVec2(frame.UV.x + frame.Size.x, frame.UV.y)))
            {
                if (!selected)
                {
                    m_RandomBrushTiles.push_back(i);
                    m_RandomBrushWeights.push_back(1.0f);
                }
                else
                {
                    int idx = std::distance(m_RandomBrushTiles.begin(),
                        std::find(m_RandomBrushTiles.begin(),
                            m_RandomBrushTiles.end(), i));

                    m_RandomBrushTiles.erase(m_RandomBrushTiles.begin() + idx);
                    m_RandomBrushWeights.erase(m_RandomBrushWeights.begin() + idx);
                }
            }

            if (selected)
            {
                int idx = std::distance(m_RandomBrushTiles.begin(),
                    std::find(m_RandomBrushTiles.begin(),
                        m_RandomBrushTiles.end(), i));

                ImGui::SameLine();
                ImGui::SetNextItemWidth(80.f);
                ImGui::SliderFloat("##weight", &m_RandomBrushWeights[idx], 0.f, 5.f);
            }

            ImGui::PopID();
        }
    }
}

//
// ──────────────────────────────────────────
//   MAIN AREA + PALETTE
// ──────────────────────────────────────────
//
void TilemapEditorPanel::RenderMainArea()
{
    ImVec2 panelSize = ImGui::GetContentRegionAvail();

    float paletteWidth = panelSize.x;

    if (!m_Asset->GetInstance()->GetAtlas().IsValid() || !m_Asset->GetInstance()->GetAtlas()->GetInstance()->GetTexture().IsValid())
        return;

    //ImGui::SameLine();
    ImGui::BeginChild("Palette", ImVec2(paletteWidth, panelSize.y), true);
    RenderPalette({ paletteWidth, panelSize.y });
    ImGui::EndChild();

    HandlePainting();
}

void TilemapEditorPanel::RenderPalette(const ImVec2& size)
{
    auto atlas = m_Asset->GetInstance()->GetAtlas().Instance();
    auto tex = atlas->GetTexture().Instance();

    int columns = (int)(size.x / 52.f);
    int index = 0;

    for (auto& fId : atlas->GetAllFrameIDs())
    {
        auto& f = atlas->GetSpriteFrame(fId);

        if (index % columns != 0) ImGui::SameLine();

        ImGui::PushID(index);
        if (ImGui::ImageButton("##s", (ImTextureID)tex->GetRendererID(),
            ImVec2(48, 48),
            ImVec2(f.UV.x, f.UV.y + f.Size.y),
            ImVec2(f.UV.x + f.Size.x, f.UV.y)))
        {
            m_SelectedTile = fId;
        }
        ImGui::PopID();
        ++index;
    }
}

//
// ──────────────────────────────────────────
//   RANDOM TILE PICK
// ──────────────────────────────────────────
//
int TilemapEditorPanel::SelectRandomTile() const
{
    if (m_RandomBrushTiles.empty())
        return -1;

    float total = 0.f;
    for (float w : m_RandomBrushWeights) total += w;

    float r = ((float)rand() / RAND_MAX) * total;

    for (size_t i = 0; i < m_RandomBrushTiles.size(); ++i)
    {
        if (r < m_RandomBrushWeights[i])
            return m_RandomBrushTiles[i];
        r -= m_RandomBrushWeights[i];
    }

    return m_RandomBrushTiles.back();
}

//
// ──────────────────────────────────────────
//   FLOOD FILL
// ──────────────────────────────────────────
//
void TilemapEditorPanel::FloodFill(int x, int y, int newTile)
{
    auto map = m_Asset->GetInstance();
    int target = map->GetTile(x, y);
    if (target == newTile) return;

    std::queue<glm::ivec2> q;
    q.push({ x, y });

    while (!q.empty())
    {
        glm::ivec2 p = q.front();
        q.pop();

        if (!map->IsValidTile(p.x, p.y)) continue;
        if (map->GetTile(p.x, p.y) != target) continue;

        map->SetTile(p.x, p.y, newTile);

        q.push({ p.x + 1, p.y });
        q.push({ p.x - 1, p.y });
        q.push({ p.x, p.y + 1 });
        q.push({ p.x, p.y - 1 });
    }
}

//
// ──────────────────────────────────────────
//   PAINTING LOGIC (all tools here)
// ──────────────────────────────────────────
//
void TilemapEditorPanel::HandlePainting()
{
    // ------------------------------------------------------------
    // Mouse inside viewport?
    // ------------------------------------------------------------
    glm::vec2 mp = GetViewport()->GetMousePosition();
    glm::vec2 vp = GetViewport()->GetSize();

    if (mp.x < 0 || mp.y < 0 || mp.x > vp.x || mp.y > vp.y)
        return;

    // ------------------------------------------------------------
    // Convert mouse → world → tile coordinates
    // ------------------------------------------------------------
    glm::vec3 world = ScreenToWorld(mp);

    float ts = m_Asset->GetInstance()->GetUnitSize();
    int tx = (int)floor(world.x / ts);
    int ty = (int)floor(world.y / ts);

    auto map = m_Asset->GetInstance();
    if (!map->IsValidTile(tx, ty))
        return;

    Input& input = ServiceLocator::Get<Input>();


    // ------------------------------------------------------------
    // FLOOD FILL
    // ------------------------------------------------------------
    if (m_Mode == TileBrushMode::FloodFill &&
        input.IsMousePressed(Mouse::ButtonLeft))
    {
        FloodFill(tx, ty, m_SelectedTile);
        return;
    }


    // ------------------------------------------------------------
    // STAMP MODE (select region or paste region)
    // ------------------------------------------------------------
    if (m_Mode == TileBrushMode::Stamp)
    {
        // Begin selection box
        if (input.IsMousePressed(Mouse::ButtonLeft))
        {
            m_StampSelecting = true;
            m_StampStart = { tx, ty };
        }

        // End selection box
        if (m_StampSelecting && input.IsMouseReleased(Mouse::ButtonLeft))
        {
            m_StampSelecting = false;

            int x0 = std::min(m_StampStart.x, tx);
            int y0 = std::min(m_StampStart.y, ty);
            int x1 = std::max(m_StampStart.x, tx);
            int y1 = std::max(m_StampStart.y, ty);

            m_StampW = x1 - x0 + 1;
            m_StampH = y1 - y0 + 1;

            m_StampBuffer.resize(m_StampW * m_StampH);

            for (int y = 0; y < m_StampH; ++y)
                for (int x = 0; x < m_StampW; ++x)
                    m_StampBuffer[y * m_StampW + x] =
                    map->GetTile(x0 + x, y0 + y);
        }

        // Paste stamp
        if (!m_StampSelecting &&
            !m_StampBuffer.empty() &&
            input.IsMouseHeld(Mouse::ButtonLeft))
        {
            for (int sy = 0; sy < m_StampH; ++sy)
                for (int sx = 0; sx < m_StampW; ++sx)
                {
                    int px = tx + sx;
                    int py = ty + sy;

                    if (!map->IsValidTile(px, py))
                        continue;

                    map->SetTile(px, py,
                        m_StampBuffer[sy * m_StampW + sx]);
                }
        }

        return;
    }


    // ------------------------------------------------------------
    // UNIVERSAL PAINT BRUSH (Paint, Erase, Random)
    // ------------------------------------------------------------

    // The brush size defines the radius in tile units.
    // size = 1 → radius = 0.5 → only the center tile
    float radiusTiles = (float)m_BrushSize - 0.5f;
    float radiusSq = radiusTiles * radiusTiles;

    // Mouse position in tile space
    float brushTx = int(world.x / ts) + ts * 0.5f;
    float brushTy = int(world.y / ts) + ts * 0.5f;

    // Sampling area (square bounding box around circle)
    // Always ±BrushSize
    int half =
        (m_BrushShape == TileBrushShape::Square)
        ? (m_BrushSize / 2)
        : m_BrushSize;

    for (int by = -half; by <= half; ++by)
    {
        for (int bx = -half; bx <= half; ++bx)
        {
            int px = tx + bx;
            int py = ty + by;

            if (!map->IsValidTile(px, py))
                continue;

            // Tile center in tile space
            float tileCx = px + 0.5f;
            float tileCy = py + 0.5f;

            // Filter based on brush shape
            bool inside = true;

            if (m_BrushShape == TileBrushShape::Circle)
            {
                float dx = tileCx - brushTx;
                float dy = tileCy - brushTy;
                inside = (dx * dx + dy * dy <= radiusSq);
            }

            // Draw preview ONLY if inside circle/square
            if (inside)
            {
                glm::vec3 center(
                    px * ts + ts * 0.5f,
                    py * ts + ts * 0.5f,
                    0.f
                );
                DEBUG_DRAW_RECT(center, glm::vec2(ts, ts),
                    glm::vec4(1, 1, 1, 1));
            }
            else
                continue;


            // --------------------------------------------------------
            // Apply paint
            // --------------------------------------------------------
            if (input.IsMouseHeld(Mouse::ButtonLeft))
            {
                int tileToPlace = m_SelectedTile;

                if (m_Mode == TileBrushMode::Random)
                    tileToPlace = SelectRandomTile();

                if (m_Mode == TileBrushMode::Erase)
                {
                    map->SetTile(px, py, -1);
                }
                else if (m_Mode == TileBrushMode::Paint ||
                    m_Mode == TileBrushMode::Random)
                {
                    map->SetTile(px, py, tileToPlace);
                }
            }
        }
    }
}
