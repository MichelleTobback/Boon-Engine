#include "Panels/SpriteAtlasEditorPanel.h"

#include <imgui.h>

#include <Renderer/Texture.h>
#include <UI/UI.h>
#include <UI/IconsFontAwesome7.h>

#include "Assets/AssetDatabase.h"

#include <algorithm>
#include <cstring>
#include <cstdio>

namespace
{
    ImU32 Col(ImGuiCol idx)
    {
        return ImGui::GetColorU32(ImGui::GetStyleColorVec4(idx));
    }

    ImU32 WithAlpha(ImGuiCol idx, float alpha)
    {
        ImVec4 c = ImGui::GetStyleColorVec4(idx);
        c.w *= alpha;
        return ImGui::GetColorU32(c);
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

    ImU32 SurfaceActive()
    {
        return Col(ImGuiCol_FrameBgActive);
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
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, ImGui::GetStyle().FrameRounding);

        ImGui::PushStyleColor(ImGuiCol_Button, active ? WithAlpha(ImGuiCol_CheckMark, 0.22f) : Col(ImGuiCol_Button));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, active ? WithAlpha(ImGuiCol_CheckMark, 0.30f) : Col(ImGuiCol_ButtonHovered));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, active ? WithAlpha(ImGuiCol_CheckMark, 0.38f) : Col(ImGuiCol_ButtonActive));

        bool pressed = ImGui::Button(label, size);

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();

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

    void DrawPanelTitle(const char* title, const char* subtitle = nullptr)
    {
        ImGui::Text("%s", title);

        if (subtitle && subtitle[0] != '\0')
        {
            ImGui::SameLine();
            ImGui::TextDisabled("%s", subtitle);
        }

        ImGui::Separator();
    }

    void DrawMutedHelp(const char* text)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::TextWrapped("%s", text);
        ImGui::PopStyleColor();
    }

    void BeginSoftCard(const char* id, ImVec2 size = ImVec2(0.0f, 0.0f))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));

        ImGui::BeginChild(
            id,
            size,
            true,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
        );
    }

    void BeginSoftCard(const char* id, float height)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));

        ImGui::BeginChild(
            id,
            ImVec2(0.0f, height),
            true,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
        );
    }

    void EndSoftCard()
    {
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
    }

    void DrawSmallStat(const char* label, const char* value)
    {
        ImGui::BeginGroup();

        ImGui::TextDisabled("%s", label);
        ImGui::Text("%s", value);

        ImGui::EndGroup();
    }

    bool Splitter(const char* id, bool vertical, float thickness, float* sizeA, float* sizeB, float minA, float minB)
    {
        ImGui::PushID(id);

        ImVec2 cursor = ImGui::GetCursorPos();
        ImVec2 size = vertical
            ? ImVec2(thickness, -1.0f)
            : ImVec2(-1.0f, thickness);

        ImGui::InvisibleButton("##splitter", size);

        bool active = ImGui::IsItemActive();
        bool hovered = ImGui::IsItemHovered();

        if (hovered || active)
            ImGui::SetMouseCursor(vertical ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS);

        if (active)
        {
            float delta = vertical ? ImGui::GetIO().MouseDelta.x : ImGui::GetIO().MouseDelta.y;

            if (*sizeA + delta >= minA && *sizeB - delta >= minB)
            {
                *sizeA += delta;
                *sizeB -= delta;
            }
        }

        ImVec2 min = ImGui::GetItemRectMin();
        ImVec2 max = ImGui::GetItemRectMax();

        ImGui::GetWindowDrawList()->AddRectFilled(
            min,
            max,
            ImGui::GetColorU32(hovered || active ? ImGuiCol_ButtonHovered : ImGuiCol_Border),
            2.0f
        );

        ImGui::PopID();
        return active;
    }

    bool EditorIconButton(
        const char* icon,
        const char* tooltip,
        bool active = false,
        ImVec2 size = ImVec2(30.0f, 30.0f))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, ImGui::GetStyle().FrameRounding);

        ImGui::PushStyleColor(ImGuiCol_Button, active ? WithAlpha(ImGuiCol_CheckMark, 0.22f) : Col(ImGuiCol_Button));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, active ? WithAlpha(ImGuiCol_CheckMark, 0.30f) : Col(ImGuiCol_ButtonHovered));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, active ? WithAlpha(ImGuiCol_CheckMark, 0.38f) : Col(ImGuiCol_ButtonActive));

        bool pressed = ImGui::Button(icon, size);

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();

        if (ImGui::IsItemHovered() && tooltip)
            ImGui::SetTooltip("%s", tooltip);

        return pressed;
    }
}

namespace BoonEditor
{
    SpriteAtlasEditorPanel::SpriteAtlasEditorPanel(EditorContext* pContext, const std::string& name)
        : AssetEditor(pContext, name)
    {
    }

    void SpriteAtlasEditorPanel::Update()
    {
    }

    bool SpriteAtlasEditorPanel::OnViewportCanvasRenderUI(const ViewportCanvasContext& context)
    {
        if (!m_Asset.IsValid())
            return false;

        std::shared_ptr<SpriteAtlas> atlas = m_Asset->GetInstance();
        if (!atlas || !atlas->GetTexture().IsValid())
            return false;

        RenderAtlasCanvas(*atlas);

        return true;
    }

    void SpriteAtlasEditorPanel::RenderToolbar()
    {
        if (!m_Asset.IsValid())
            return;

        std::shared_ptr<SpriteAtlas> atlas = m_Asset->GetInstance();
        if (!atlas)
            return;

        constexpr ImVec2 buttonSize{ 30.0f, 30.0f };

        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Sprite Atlas Editor");

        ImGui::SameLine();
        ImGui::TextDisabled("| Slice sprites, define frames, build clips");

        ImGui::Spacing();

        if (EditorIconButton(ICON_FA_FLOPPY_DISK, "Save", false, buttonSize))
            AssetDatabase::Get().Export<SpriteAtlasAsset>(m_Asset);

        ImGui::SameLine(0.0f, 10.0f);
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine(0.0f, 10.0f);

        if (EditorIconButton(ICON_FA_ARROW_POINTER, "Select Mode", m_Mode == AtlasEditorMode::Select, buttonSize))
            m_Mode = AtlasEditorMode::Select;

        ImGui::SameLine(0.0f, 4.0f);

        if (EditorIconButton(ICON_FA_SCISSORS, "Slice Mode", m_Mode == AtlasEditorMode::Slice, buttonSize))
            m_Mode = AtlasEditorMode::Slice;

        ImGui::SameLine(0.0f, 10.0f);
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        UI::SameLine();

        AssetHandle tex = atlas->GetTexture();

        ImGui::SetNextItemWidth(260.0f);



        if (UI::AssetRef("Texture", tex, AssetType::Texture))
        {
            atlas->SetTexture(AssetRef<Texture2DAsset>(tex));
            m_SelectedSprite = -1;
            m_SelectedGridCells.clear();
            RestartPreview();
        }

        ImGui::SameLine(0.0f, 12.0f);
        ImGui::TextDisabled("MMB Pan  |  Wheel Zoom  |  Drag frames to timeline");
    }

    void SpriteAtlasEditorPanel::RenderMainArea()
    {
        std::shared_ptr<SpriteAtlas> atlas = m_Asset->GetInstance();
        if (!atlas)
            return;

        AssetHandle tex = atlas->GetTexture();

        // ─────────────────────────────────────────────
        // Asset setup header
        // ─────────────────────────────────────────────
        //BeginEditorPanel("##sprite_atlas_asset_header", ImVec2(0.0f, 74.0f));
        //
        //ImGui::Text("Texture Source");
        //ImGui::SameLine();
        //ImGui::TextDisabled("Choose the texture used by this atlas.");
        //
        //ImGui::Spacing();
        //
        //if (UI::AssetRef("Texture", tex, AssetType::Texture))
        //{
        //    atlas->SetTexture(AssetRef<Texture2DAsset>(tex));
        //    m_SelectedSprite = -1;
        //    m_SelectedGridCells.clear();
        //    RestartPreview();
        //}
        //
        //EndEditorPanel();

        if (!atlas->GetTexture().IsValid())
        {
            ImGui::Spacing();

            BeginEditorPanel("##sprite_atlas_missing_texture", ImVec2(0.0f, 180.0f));
            ImGui::Text("No texture assigned");
            ImGui::Spacing();
            DrawMutedHelp("Assign a texture above to start slicing sprites and creating animation clips.");
            EndEditorPanel();
            return;
        }

        ImVec2 avail = ImGui::GetContentRegionAvail();

        const float gap = 8.0f;
        const float splitterSize = 5.0f;

        static float timelineHeight = 176.0f;

        timelineHeight = std::clamp(timelineHeight, 120.0f, std::max(120.0f, avail.y - 220.0f));

        float upperHeight = avail.y - timelineHeight - splitterSize - gap;
        upperHeight = std::max(220.0f, upperHeight);

        static float inspectorRatio = 0.36f;

        const float availableSplitWidth =
            avail.x - splitterSize - gap;

        const float minPreviewWidth = 260.0f;
        const float minInspectorWidth = 280.0f;

        float inspectorWidth =
            availableSplitWidth * inspectorRatio;

        inspectorWidth = std::clamp(
            inspectorWidth,
            minInspectorWidth,
            std::max(minInspectorWidth, availableSplitWidth - minPreviewWidth));

        float previewWidth =
            availableSplitWidth - inspectorWidth;

        previewWidth = std::clamp(
            previewWidth,
            minPreviewWidth,
            std::max(minPreviewWidth, availableSplitWidth - minInspectorWidth));


        ImGui::BeginChild(
            "##sprite_atlas_upper_layout",
            ImVec2(0.0f, upperHeight),
            false,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
        );
        
        BeginEditorPanel("##sprite_atlas_preview_panel", ImVec2(previewWidth, upperHeight));
        DrawPanelTitle(
            "Preview",
            atlas->IsValidClip(m_SelectedClip) ? atlas->GetClip(m_SelectedClip).Name.c_str() : "No clip selected"
        );
        RenderAnimationPreview(*atlas);
        EndEditorPanel();

        ImGui::SameLine(0.0f, gap);

        float sideWidth = inspectorWidth;
        Splitter("##preview_inspector_splitter", true, splitterSize, &previewWidth, &sideWidth, 260.0f, 280.0f);
        inspectorWidth = sideWidth;

        if (availableSplitWidth > 0.0f)
        {
            inspectorRatio =
                std::clamp(inspectorWidth / availableSplitWidth, 0.20f, 0.65f);
        }

        ImGui::SameLine(0.0f, gap);

        BeginEditorPanel("##sprite_atlas_side_panel", ImVec2(inspectorWidth, upperHeight));
        DrawPanelTitle("Atlas Editor", m_Mode == AtlasEditorMode::Slice ? "Slice mode" : "Select mode");

        if (ImGui::BeginTabBar("##sprite_atlas_tabs", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Sprites"))
            {
                ImGui::BeginChild(
                    "##sprite_tab_scroll",
                    ImVec2(0.0f, 0.0f),
                    false,
                    ImGuiWindowFlags_AlwaysVerticalScrollbar
                );

                RenderInspector(*atlas);

                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Clips"))
            {
                ImGui::BeginChild(
                    "##clips_tab_scroll",
                    ImVec2(0.0f, 0.0f),
                    false,
                    ImGuiWindowFlags_AlwaysVerticalScrollbar
                );

                RenderClipEditor(*atlas);

                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        EndEditorPanel();

        ImGui::EndChild();

        ImGui::Spacing();

        float newTimelineHeight = timelineHeight;
        float newUpperHeight = upperHeight;

        Splitter(
            "##upper_timeline_splitter",
            false,
            splitterSize,
            &newUpperHeight,
            &newTimelineHeight,
            220.0f,
            120.0f
        );

        timelineHeight = newTimelineHeight;

        BeginEditorPanel("##sprite_atlas_timeline_panel", ImVec2(0.0f, timelineHeight));
        RenderTimeline(*atlas);
        EndEditorPanel();
    }


    void SpriteAtlasEditorPanel::RenderAnimationPreview(SpriteAtlas& atlas)
    {
        ImTextureID textureId{};
        glm::vec2 textureSize{ 0.0f };

        if (!GetTextureInfo(atlas, textureId, textureSize))
            return;

        ImVec2 avail = ImGui::GetContentRegionAvail();

        ImGui::BeginChild(
            "##animation_preview_canvas",
            avail,
            false,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
        );

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        ImVec2 canvasMin = ImGui::GetCursorScreenPos();
        ImVec2 canvasSize = ImGui::GetContentRegionAvail();
        ImVec2 canvasMax(canvasMin.x + canvasSize.x, canvasMin.y + canvasSize.y);

        drawList->AddRectFilled(
            canvasMin,
            canvasMax,
            Surface(),
            ImGui::GetStyle().WindowRounding
        );

        if (!atlas.IsValidClip(m_SelectedClip))
        {
            drawList->AddText(
                ImVec2(canvasMin.x + 16.0f, canvasMin.y + 16.0f),
                TextMuted(),
                "Select an animation clip to preview it."
            );

            ImGui::Dummy(canvasSize);
            ImGui::EndChild();
            return;
        }

        SpriteAnimClip& clip = atlas.GetClip(m_SelectedClip);

        if (clip.Frames.empty())
        {
            drawList->AddText(
                ImVec2(canvasMin.x + 16.0f, canvasMin.y + 16.0f),
                TextMuted(),
                "This clip has no frames yet."
            );

            ImGui::Dummy(canvasSize);
            ImGui::EndChild();
            return;
        }

        // Controls FIRST, before updating preview state.
        ImGui::SetCursorScreenPos(ImVec2(canvasMin.x + 12.0f, canvasMin.y + 12.0f));

        if (EditorButton(
            m_PreviewPlaying ? "Pause" : "Resume",
            false,
            ImVec2(78.0f, 26.0f)))
        {
            m_PreviewPlaying = !m_PreviewPlaying;
        }

        ImGui::SameLine();

        if (EditorButton("Restart", false, ImVec2(78.0f, 26.0f)))
        {
            m_PreviewFrame = 0;
            m_PreviewTimer = 0.0f;
        }

        ImGui::SameLine();

        ImGui::TextDisabled(
            "Frame %d / %d",
            m_PreviewFrame + 1,
            static_cast<int>(clip.Frames.size())
        );

        UpdateAnimationPreview(atlas);

        m_PreviewFrame = std::clamp(
            m_PreviewFrame,
            0,
            static_cast<int>(clip.Frames.size()) - 1
        );

        int frameId = clip.Frames[m_PreviewFrame];

        if (!atlas.Exists(frameId))
        {
            drawList->AddText(
                ImVec2(canvasMin.x + 16.0f, canvasMin.y + 56.0f),
                TextMuted(),
                "Current preview frame is missing."
            );

            ImGui::Dummy(canvasSize);
            ImGui::EndChild();
            return;
        }

        const SpriteFrame& frame = atlas.GetSpriteFrame(frameId);

        constexpr float reservedTop = 56.0f;
        constexpr float reservedBottom = 34.0f;

        float availableW = canvasSize.x;
        float availableH = canvasSize.y - reservedTop - reservedBottom;

        glm::vec2 framePixelSize = SizeToPixel(frame, textureSize);

        float frameAspect = 1.0f;
        if (framePixelSize.y > 0.0f)
            frameAspect = framePixelSize.x / framePixelSize.y;

        float maxPreviewW = availableW * 0.55f;
        float maxPreviewH = availableH * 0.85f;

        float previewW = maxPreviewW;
        float previewH = previewW / frameAspect;

        if (previewH > maxPreviewH)
        {
            previewH = maxPreviewH;
            previewW = previewH * frameAspect;
        }

        previewW = std::max(previewW, 32.0f);
        previewH = std::max(previewH, 32.0f);

        ImVec2 imageMin(
            canvasMin.x + canvasSize.x * 0.5f - previewW * 0.5f,
            canvasMin.y + reservedTop + availableH * 0.5f - previewH * 0.5f
        );

        ImVec2 imageMax(
            imageMin.x + previewW,
            imageMin.y + previewH
        );

        drawList->AddImage(
            textureId,
            imageMin,
            imageMax,
            ImVec2(frame.UV.x, frame.UV.y + frame.Size.y),
            ImVec2(frame.UV.x + frame.Size.x, frame.UV.y),
            IM_COL32_WHITE
        );

        float effectiveFPS = clip.FPS * clip.Speed;

        float effectiveFrameTime =
            effectiveFPS > 0.0f
            ? 1.0f / effectiveFPS
            : 0.0f;

        char status[256]{};

        std::snprintf(
            status,
            sizeof(status),
            "%s  |  id:%d  |  speed %.2fx  |  FPS %.1f  |  %.3fs/frame",
            m_PreviewPlaying ? "Playing" : "Paused",
            frameId,
            clip.Speed,
            effectiveFPS,
            effectiveFrameTime
        );

        drawList->AddText(
            ImVec2(canvasMin.x + 16.0f, canvasMax.y - 26.0f),
            TextMuted(),
            status
        );

        ImGui::Dummy(canvasSize);
        ImGui::EndChild();
    }

    void SpriteAtlasEditorPanel::RenderAtlasCanvas(SpriteAtlas& atlas)
    {
        ImTextureID textureId{};
        glm::vec2 textureSize{ 0.0f };

        if (!GetTextureInfo(atlas, textureId, textureSize))
            return;

        m_AtlasCanvas.Begin("SpriteAtlasCanvas");

        m_AtlasCanvas.DrawImage(
            textureId,
            { 0.0f, 0.0f },
            textureSize,
            { 0.0f, 1.0f },
            { 1.0f, 0.0f }
        );

        m_AtlasCanvas.DrawRect(
            { 0.0f, 0.0f },
            textureSize,
            Border(),
            1.0f
        );

        glm::vec2 mouse = m_AtlasCanvas.GetMouseCanvas();
        int hoveredFrame = HitTestFrame(atlas, mouse);

        if (m_AtlasCanvas.IsHovered() && m_Mode == AtlasEditorMode::Select)
        {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                m_SelectedSprite = hoveredFrame;

            if (m_SelectedSprite != -1 &&
                ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                int payloadId = m_SelectedSprite;

                ImGui::SetDragDropPayload(
                    "SPRITE_ATLAS_FRAME_ID",
                    &payloadId,
                    sizeof(int)
                );

                ImGui::Text("Frame id:%d", payloadId);
                ImGui::EndDragDropSource();
            }
        }

        if (m_Mode == AtlasEditorMode::Slice)
            RenderSliceGrid(atlas);

        RenderExistingFrames(atlas);

        m_AtlasCanvas.End();
    }

    void SpriteAtlasEditorPanel::RenderExistingFrames(SpriteAtlas& atlas)
    {
        ImTextureID textureId{};
        glm::vec2 textureSize{ 0.0f };

        if (!GetTextureInfo(atlas, textureId, textureSize))
            return;

        int hovered = -1;

        if (m_AtlasCanvas.IsHovered())
            hovered = HitTestFrame(atlas, m_AtlasCanvas.GetMouseCanvas());

        for (int id : atlas.GetAllFrameIDs())
        {
            if (!atlas.Exists(id))
                continue;

            const SpriteFrame& frame = atlas.GetSpriteFrame(id);

            glm::vec2 pos = UVToPixel(frame, textureSize);
            glm::vec2 size = SizeToPixel(frame, textureSize);

            ImU32 color = IM_COL32(255, 255, 255, 120);
            float thickness = 1.0f;

            if (id == m_SelectedSprite)
            {
                color = Accent(1.0f);
                thickness = 2.5f;
            }
            else if (id == hovered)
            {
                color = Accent(0.75f);
                thickness = 2.0f;
            }

            m_AtlasCanvas.DrawRect(pos, size, color, thickness);
        }
    }

    void SpriteAtlasEditorPanel::RenderSliceGrid(SpriteAtlas& atlas)
    {
        ImTextureID textureId{};
        glm::vec2 textureSize{ 0.0f };

        if (!GetTextureInfo(atlas, textureId, textureSize))
            return;

        m_GridTileSize.x = std::max(1, m_GridTileSize.x);
        m_GridTileSize.y = std::max(1, m_GridTileSize.y);
        m_Cols = std::max(1, m_Cols);
        m_Rows = std::max(1, m_Rows);

        const bool useCellSize = m_GridMode == GridMode::Cellsize;

        const int xCount = useCellSize
            ? std::max(1, static_cast<int>(textureSize.x) / m_GridTileSize.x)
            : m_Cols;

        const int yCount = useCellSize
            ? std::max(1, static_cast<int>(textureSize.y) / m_GridTileSize.y)
            : m_Rows;

        const glm::vec2 cellSize = useCellSize
            ? glm::vec2(static_cast<float>(m_GridTileSize.x), static_cast<float>(m_GridTileSize.y))
            : glm::vec2(textureSize.x / static_cast<float>(m_Cols), textureSize.y / static_cast<float>(m_Rows));

        glm::vec2 mouse = m_AtlasCanvas.GetMouseCanvas();

        int hoveredCell = -1;

        if (mouse.x >= 0.0f &&
            mouse.y >= 0.0f &&
            mouse.x < textureSize.x &&
            mouse.y < textureSize.y)
        {
            int mx = static_cast<int>(mouse.x / cellSize.x);
            int my = static_cast<int>(mouse.y / cellSize.y);

            if (mx >= 0 && my >= 0 && mx < xCount && my < yCount)
                hoveredCell = mx + my * xCount;
        }

        if (m_AtlasCanvas.IsHovered() &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
            hoveredCell >= 0)
        {
            if (m_SelectedGridCells.find(hoveredCell) == m_SelectedGridCells.end())
                m_SelectedGridCells.insert(hoveredCell);
            else
                m_SelectedGridCells.erase(hoveredCell);
        }

        for (int y = 0; y < yCount; ++y)
        {
            for (int x = 0; x < xCount; ++x)
            {
                const int index = x + y * xCount;

                glm::vec2 pos = {
                    static_cast<float>(x) * cellSize.x,
                    static_cast<float>(y) * cellSize.y
                };

                ImU32 color = IM_COL32(255, 255, 255, 70);
                float thickness = 1.0f;

                if (m_SelectedGridCells.find(index) != m_SelectedGridCells.end())
                {
                    color = Accent(1.0f);
                    thickness = 2.0f;
                }
                else if (index == hoveredCell)
                {
                    color = Accent(0.7f);
                    thickness = 2.0f;
                }

                m_AtlasCanvas.DrawRect(pos, cellSize, color, thickness);
            }
        }
    }

    void SpriteAtlasEditorPanel::RenderInspector(SpriteAtlas& atlas)
    {
        ImTextureID textureId{};
        glm::vec2 textureSize{ 0.0f };
        GetTextureInfo(atlas, textureId, textureSize);

        BeginSoftCard("##sprite_summary_card", ImVec2(0.0f, 78.0f));
        ImGui::Text("Sprites");
        ImGui::SameLine();
        ImGui::TextDisabled("%d frames", static_cast<int>(atlas.GetAllFrameIDs().size()));

        ImGui::Spacing();

        if (m_Mode == AtlasEditorMode::Select)
            DrawMutedHelp("Select a frame in the viewport, edit its pixel rectangle, or drag it into the timeline.");
        else
            DrawMutedHelp("Select cells in the viewport, then create frames from the selected slice cells.");

        EndSoftCard();

        ImGui::Spacing();

        BeginSoftCard("##frame_inspector_card", atlas.Exists(m_SelectedSprite) ? 190.0f : 96.0f);
        ImGui::Text("Frame Inspector");
        ImGui::Separator();

        if (!atlas.Exists(m_SelectedSprite))
        {
            DrawMutedHelp("No frame selected. Click a frame in the atlas viewport to inspect it.");
        }
        else
        {
            SpriteFrame frame = atlas.GetSpriteFrame(m_SelectedSprite);

            char idBuffer[32]{};
            std::snprintf(idBuffer, sizeof(idBuffer), "%d", m_SelectedSprite);
            DrawSmallStat("Stable ID", idBuffer);

            glm::ivec2 pixelPos = UVToPixel(frame, textureSize);
            glm::ivec2 pixelSize = SizeToPixel(frame, textureSize);

            bool changed = false;

            changed |= UI::DragInt2("Position", pixelPos, 0, 99999);
            changed |= UI::DragInt2("Size", pixelSize, 1, 99999);

            if (changed)
            {
                pixelPos.x = std::clamp(pixelPos.x, 0, (int)textureSize.x);
                pixelPos.y = std::clamp(pixelPos.y, 0, (int)textureSize.y);

                pixelSize.x = std::clamp(pixelSize.x, 1, (int)textureSize.x - pixelPos.x);
                pixelSize.y = std::clamp(pixelSize.y, 1, (int)textureSize.y - pixelPos.y);

                SpriteFrame updated = PixelToFrame(pixelPos, pixelSize, textureSize);
                atlas.SetSpriteFrame(m_SelectedSprite, updated);
            }

            ImGui::Spacing();

            if (EditorButton("Add to Clip", false, ImVec2(96.0f, 26.0f)))
                AddSelectedFrameToCurrentClip(atlas);

            ImGui::SameLine();

            if (EditorButton("Delete", false, ImVec2(72.0f, 26.0f)))
            {
                atlas.RemoveSpriteFrame(m_SelectedSprite);
                m_SelectedSprite = -1;
            }
        }

        EndSoftCard();

        ImGui::Spacing();

        BeginSoftCard("##sprite_browser_card", ImVec2(0.0f, 178.0f));
        ImGui::Text("Sprite Browser");
        ImGui::SameLine();
        ImGui::TextDisabled("drag frames into the timeline");
        ImGui::Separator();

        if (atlas.GetAllFrameIDs().empty())
        {
            DrawMutedHelp("No frames have been created yet. Switch to Slice mode and create frames from the grid.");
        }
        else
        {
            ImGui::BeginChild(
                "##sprite_frame_browser_scroll",
                ImVec2(0.0f, 120.0f),
                false,
                ImGuiWindowFlags_HorizontalScrollbar
            );

            constexpr ImVec2 cardSize{ 72.0f, 96.0f };
            constexpr ImVec2 thumbSize{ 52.0f, 52.0f };
            constexpr float spacing = 8.0f;

            ImDrawList* drawList = ImGui::GetWindowDrawList();
            const std::vector<int> ids = atlas.GetAllFrameIDs();

            for (int i = 0; i < static_cast<int>(ids.size()); ++i)
            {
                const int id = ids[i];

                if (!atlas.Exists(id))
                    continue;

                ImGui::PushID(id);

                ImVec2 pos = ImGui::GetCursorScreenPos();

                ImGui::InvisibleButton("##sprite_card", cardSize);

                bool hovered = ImGui::IsItemHovered();
                bool selected = id == m_SelectedSprite;

                if (ImGui::IsItemClicked())
                    m_SelectedSprite = id;

                if (ImGui::BeginDragDropSource())
                {
                    int payloadId = id;

                    ImGui::SetDragDropPayload(
                        "SPRITE_ATLAS_FRAME_ID",
                        &payloadId,
                        sizeof(int)
                    );

                    ImGui::Text("Frame id:%d", payloadId);
                    ImGui::EndDragDropSource();
                }

                ImVec2 max(pos.x + cardSize.x, pos.y + cardSize.y);

                ImU32 bg =
                    selected ? Accent(0.22f) :
                    hovered ? SurfaceHover() :
                    Surface();

                ImU32 border =
                    selected || hovered ? Accent(0.95f) : Border();

                drawList->AddRectFilled(pos, max, bg, ImGui::GetStyle().FrameRounding);
                drawList->AddRect(pos, max, border, ImGui::GetStyle().FrameRounding, 0, selected ? 2.0f : 1.0f);

                ImVec2 imageMin(pos.x + 10.0f, pos.y + 9.0f);
                ImVec2 imageMax(imageMin.x + thumbSize.x, imageMin.y + thumbSize.y);

                drawList->AddRectFilled(imageMin, imageMax, IM_COL32(12, 13, 14, 255), ImGui::GetStyle().FrameRounding);

                const SpriteFrame& frame = atlas.GetSpriteFrame(id);

                drawList->AddImage(
                    textureId,
                    imageMin,
                    imageMax,
                    ImVec2(frame.UV.x, frame.UV.y + frame.Size.y),
                    ImVec2(frame.UV.x + frame.Size.x, frame.UV.y),
                    IM_COL32_WHITE
                );

                drawList->AddRect(imageMin, imageMax, Border(), ImGui::GetStyle().FrameRounding);

                char label[32]{};
                std::snprintf(label, sizeof(label), "id:%d", id);

                drawList->AddText(
                    ImVec2(pos.x + 10.0f, pos.y + 68.0f),
                    selected ? Col(ImGuiCol_Text) : TextMuted(),
                    label
                );

                ImGui::PopID();

                if (i + 1 < static_cast<int>(ids.size()))
                    ImGui::SameLine(0.0f, spacing);
            }

            ImGui::EndChild();
        }

        EndSoftCard();

        if (m_Mode == AtlasEditorMode::Slice)
        {
            ImGui::Spacing();

            BeginSoftCard("##slice_grid_card", 190.0f);
            ImGui::Text("Slice Grid");
            ImGui::Separator();

            int gridMode = static_cast<int>(m_GridMode);
            const char* modes[] = { "Cell Size", "Rows / Columns" };

            if (UI::Combo("Grid Mode", gridMode, modes, 2))
                m_GridMode = static_cast<GridMode>(gridMode);

            if (m_GridMode == GridMode::Cellsize)
            {
                UI::DragInt2("Tile Size", m_GridTileSize, 1, 4096);
            }
            else
            {
                UI::DragInt("Columns", m_Cols, 1, 4096);
                UI::SameLine();
                UI::DragInt("Rows", m_Rows, 1, 4096);
            }

            ImGui::Spacing();

            if (EditorButton("Create", false, ImVec2(78.0f, 26.0f)))
                AddSelectedGridCellsAsFrames(atlas);

            ImGui::SameLine();

            if (EditorButton("Clear", false, ImVec2(72.0f, 26.0f)))
                m_SelectedGridCells.clear();

            ImGui::Spacing();
            ImGui::TextDisabled("%d selected cells", static_cast<int>(m_SelectedGridCells.size()));

            EndSoftCard();
        }
    }


    void SpriteAtlasEditorPanel::RenderClipEditor(SpriteAtlas& atlas)
    {
        BeginSoftCard("##clip_summary_card", ImVec2(0.0f, 76.0f));
        ImGui::Text("Animation Clips");
        ImGui::SameLine();
        ImGui::TextDisabled("%d clips", static_cast<int>(atlas.GetClipCount()));

        ImGui::Spacing();
        DrawMutedHelp("Create clips, edit playback settings, then drag sprite frames into the timeline below.");

        EndSoftCard();

        ImGui::Spacing();

        BeginSoftCard("##clip_list_card", ImVec2(0.0f, 188.0f));

        if (EditorButton("New Clip", false, ImVec2(86.0f, 26.0f)))
        {
            SpriteAnimClip clip{};
            clip.Name = "New Clip";
            clip.Speed = 1.0f;
            clip.pAtlas = &atlas;

            atlas.AddClip(clip);
            m_SelectedClip = static_cast<int>(atlas.GetClipCount()) - 1;
        }

        ImGui::SameLine();

        const bool canRemove = atlas.IsValidClip(m_SelectedClip);

        if (EditorButton("Remove", false, ImVec2(78.0f, 26.0f)) && canRemove)
        {
            atlas.RemoveClip(m_SelectedClip);

            if (m_SelectedClip >= static_cast<int>(atlas.GetClipCount()))
                m_SelectedClip = static_cast<int>(atlas.GetClipCount()) - 1;

            RestartPreview();
        }

        ImGui::Spacing();

        ImGui::BeginChild("##clip_list", ImVec2(0.0f, 116.0f), true);

        if (atlas.GetClipCount() == 0)
        {
            DrawMutedHelp("No clips yet. Create one to start building an animation.");
        }

        for (int i = 0; i < static_cast<int>(atlas.GetClipCount()); ++i)
        {
            SpriteAnimClip& clip = atlas.GetClip(i);

            std::string label = clip.Name.empty()
                ? "Clip " + std::to_string(i)
                : clip.Name;

            label += "  (" + std::to_string(static_cast<int>(clip.Frames.size())) + " frames)";

            if (ImGui::Selectable(label.c_str(), m_SelectedClip == i))
            {
                m_SelectedClip = i;
                RestartPreview();
            }
        }

        ImGui::EndChild();
        EndSoftCard();

        if (!atlas.IsValidClip(m_SelectedClip))
            return;

        ImGui::Spacing();

        BeginSoftCard("##clip_settings_card", 190.0f);

        SpriteAnimClip& selected = atlas.GetClip(m_SelectedClip);

        ImGui::Text("Clip Settings");
        ImGui::Separator();

        char nameBuffer[128]{};
        std::strncpy(nameBuffer, selected.Name.c_str(), sizeof(nameBuffer) - 1);

        if (UI::Field("Name", selected.Name))
        {
        }

        int fps = static_cast<int>(selected.FPS);
        if (UI::DragInt("FPS", fps, 1, 60))
            selected.FPS = static_cast<float>(fps);

        UI::DragFloat("Speed", selected.Speed, 0.01f, 20.0f, 0.01f);

        ImGui::Spacing();

        if (EditorButton("Add Frame", false, ImVec2(92.0f, 26.0f)))
            AddSelectedFrameToCurrentClip(atlas);

        ImGui::SameLine();

        if (EditorButton("Clear", false, ImVec2(72.0f, 26.0f)))
        {
            selected.Frames.clear();
            RestartPreview();
        }

        EndSoftCard();
    }


    void SpriteAtlasEditorPanel::RenderTimeline(SpriteAtlas& atlas)
    {
        if (!atlas.IsValidClip(m_SelectedClip))
        {
            ImGui::TextDisabled("Select or create a clip to edit its timeline.");
            return;
        }

        SpriteAnimClip& clip = atlas.GetClip(m_SelectedClip);

        ImTextureID textureId{};
        glm::vec2 textureSize{ 0.0f };

        if (!GetTextureInfo(atlas, textureId, textureSize))
            return;

        constexpr ImVec2 cardSize{ 86.0f, 108.0f };
        constexpr ImVec2 thumbSize{ 64.0f, 64.0f };
        constexpr float spacing = 10.0f;
        constexpr float timelineHeight = 124.0f;

        ImGui::Text("Timeline");
        ImGui::SameLine();
        ImGui::TextDisabled("Clip:");
        ImGui::SameLine();
        ImGui::Text("%s", clip.Name.empty() ? "<unnamed>" : clip.Name.c_str());
        ImGui::SameLine();
        ImGui::TextDisabled("%d frames", static_cast<int>(clip.Frames.size()));

        ImGui::Separator();

        ImGui::BeginChild(
            "##timeline_scroll",
            ImVec2(0.0f, timelineHeight),
            true,
            ImGuiWindowFlags_HorizontalScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse
        );

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        auto DrawCard = [&](const ImVec2& pos, int frameId, int labelIndex, bool ghost, bool hovered)
            {
                ImVec2 max(pos.x + cardSize.x, pos.y + cardSize.y);

                ImU32 bg = ghost
                    ? Accent(0.18f)
                    : hovered
                    ? SurfaceHover()
                    : Surface();

                ImU32 border = ghost || hovered
                    ? Accent(0.95f)
                    : Border();

                drawList->AddRectFilled(pos, max, bg, ImGui::GetStyle().WindowRounding);
                drawList->AddRect(pos, max, border, ImGui::GetStyle().WindowRounding, 0, ghost || hovered ? 2.0f : 1.0f);

                ImVec2 imageMin(pos.x + 11.0f, pos.y + 9.0f);
                ImVec2 imageMax(imageMin.x + thumbSize.x, imageMin.y + thumbSize.y);

                drawList->AddRectFilled(imageMin, imageMax, IM_COL32(12, 13, 14, 255), ImGui::GetStyle().FrameRounding);

                if (atlas.Exists(frameId))
                {
                    const SpriteFrame& frame = atlas.GetSpriteFrame(frameId);

                    drawList->AddImage(
                        textureId,
                        imageMin,
                        imageMax,
                        ImVec2(frame.UV.x, frame.UV.y + frame.Size.y),
                        ImVec2(frame.UV.x + frame.Size.x, frame.UV.y),
                        ghost ? IM_COL32(255, 255, 255, 145) : IM_COL32_WHITE
                    );
                }
                else if (ghost)
                {
                    drawList->AddText(
                        ImVec2(imageMin.x + 13.0f, imageMin.y + 24.0f),
                        Col(ImGuiCol_Text),
                        "Drop"
                    );
                }

                drawList->AddRect(imageMin, imageMax, Border(), ImGui::GetStyle().FrameRounding);

                char label[64]{};

                if (ghost)
                    std::snprintf(label, sizeof(label), "Drop here");
                else
                    std::snprintf(label, sizeof(label), "#%d  id:%d", labelIndex, frameId);

                drawList->AddText(
                    ImVec2(pos.x + 11.0f, pos.y + 80.0f),
                    ghost ? Col(ImGuiCol_Text) : Col(ImGuiCol_Text),
                    label
                );
            };

        const ImGuiPayload* payload = ImGui::GetDragDropPayload();

        int draggedExistingIndex = -1;
        int draggedAtlasFrame = -1;

        if (payload)
        {
            if (payload->IsDataType("SPRITE_CLIP_FRAME_INDEX"))
                draggedExistingIndex = *static_cast<const int*>(payload->Data);

            if (payload->IsDataType("SPRITE_ATLAS_FRAME_ID"))
                draggedAtlasFrame = *static_cast<const int*>(payload->Data);
        }

        const bool draggingExisting =
            draggedExistingIndex >= 0 &&
            draggedExistingIndex < static_cast<int>(clip.Frames.size());

        const bool draggingAtlas =
            draggedAtlasFrame >= 0 &&
            atlas.Exists(draggedAtlasFrame);

        const bool dragging = draggingExisting || draggingAtlas;

        ImVec2 start = ImGui::GetCursorScreenPos();
        ImVec2 mouse = ImGui::GetMousePos();

        int visibleCount = static_cast<int>(clip.Frames.size());
        if (draggingExisting)
            visibleCount--;

        int previewInsert = -1;

        if (dragging)
        {
            float step = cardSize.x + spacing;
            float localX = mouse.x - start.x;

            previewInsert = static_cast<int>((localX + step * 0.5f) / step);
            previewInsert = std::clamp(previewInsert, 0, visibleCount);
        }

        int removeIndex = -1;
        int visualIndex = 0;
        int sourceIndex = 0;

        if (clip.Frames.empty() && !dragging)
        {
            ImGui::InvisibleButton("##empty_timeline", ImVec2(320.0f, cardSize.y));

            ImVec2 min = ImGui::GetItemRectMin();
            ImVec2 max = ImGui::GetItemRectMax();

            drawList->AddRectFilled(min, max, Surface(), ImGui::GetStyle().WindowRounding);
            drawList->AddRect(min, max, Border(), ImGui::GetStyle().WindowRounding);

            drawList->AddText(
                ImVec2(min.x + 18.0f, min.y + 42.0f),
                TextMuted(),
                "Drag frames from the atlas here"
            );

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* dropPayload =
                    ImGui::AcceptDragDropPayload("SPRITE_ATLAS_FRAME_ID"))
                {
                    int frameId = *static_cast<const int*>(dropPayload->Data);

                    if (atlas.Exists(frameId))
                        clip.Frames.push_back(frameId);
                }

                ImGui::EndDragDropTarget();
            }

            ImGui::EndChild();
            return;
        }

        for (int slot = 0; slot <= visibleCount; ++slot)
        {
            if (dragging && slot == previewInsert)
            {
                int ghostFrame = -1;

                if (draggingExisting)
                    ghostFrame = clip.Frames[draggedExistingIndex];
                else if (draggingAtlas)
                    ghostFrame = draggedAtlasFrame;

                ImVec2 pos = ImGui::GetCursorScreenPos();

                ImGui::InvisibleButton("##ghost_slot", cardSize);
                DrawCard(pos, ghostFrame, -1, true, false);

                if (slot < visibleCount)
                    ImGui::SameLine(0.0f, spacing);
            }

            if (slot == visibleCount)
                break;

            while (draggingExisting && sourceIndex == draggedExistingIndex)
                sourceIndex++;

            if (sourceIndex >= static_cast<int>(clip.Frames.size()))
                break;

            const int currentSourceIndex = sourceIndex;
            const int frameId = clip.Frames[currentSourceIndex];

            ImGui::PushID(currentSourceIndex);

            ImVec2 pos = ImGui::GetCursorScreenPos();

            ImGui::InvisibleButton("##timeline_card", cardSize);

            bool hovered = ImGui::IsItemHovered();

            if (ImGui::BeginDragDropSource())
            {
                int payloadIndex = currentSourceIndex;

                ImGui::SetDragDropPayload(
                    "SPRITE_CLIP_FRAME_INDEX",
                    &payloadIndex,
                    sizeof(int)
                );

                ImGui::Text("Move frame #%d", currentSourceIndex);
                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginPopupContextItem("FrameContextMenu"))
            {
                if (ImGui::MenuItem("Remove"))
                    removeIndex = currentSourceIndex;

                ImGui::EndPopup();
            }

            DrawCard(pos, frameId, visualIndex, false, hovered);

            ImGui::PopID();

            sourceIndex++;
            visualIndex++;

            if (slot + 1 < visibleCount || (dragging && previewInsert == visibleCount))
                ImGui::SameLine(0.0f, spacing);
        }

        ImVec2 dropAreaMin = start;
        ImVec2 dropAreaMax(
            start.x + std::max(
                340.0f,
                (visibleCount + (dragging ? 1 : 0)) * (cardSize.x + spacing)
            ),
            start.y + cardSize.y
        );

        ImGui::SetCursorScreenPos(dropAreaMin);
        ImGui::InvisibleButton(
            "##timeline_drop_area",
            ImVec2(dropAreaMax.x - dropAreaMin.x, dropAreaMax.y - dropAreaMin.y)
        );

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* dropPayload =
                ImGui::AcceptDragDropPayload("SPRITE_ATLAS_FRAME_ID"))
            {
                int frameId = *static_cast<const int*>(dropPayload->Data);

                if (atlas.Exists(frameId))
                {
                    int insert = previewInsert < 0
                        ? static_cast<int>(clip.Frames.size())
                        : previewInsert;

                    insert = std::clamp(insert, 0, static_cast<int>(clip.Frames.size()));

                    clip.Frames.insert(clip.Frames.begin() + insert, frameId);
                }
            }

            if (const ImGuiPayload* dropPayload =
                ImGui::AcceptDragDropPayload("SPRITE_CLIP_FRAME_INDEX"))
            {
                int from = *static_cast<const int*>(dropPayload->Data);

                if (from >= 0 && from < static_cast<int>(clip.Frames.size()))
                {
                    int to = previewInsert < 0 ? from : previewInsert;

                    int moved = clip.Frames[from];
                    clip.Frames.erase(clip.Frames.begin() + from);

                    if (from < to)
                        to--;

                    to = std::clamp(to, 0, static_cast<int>(clip.Frames.size()));

                    clip.Frames.insert(clip.Frames.begin() + to, moved);
                }
            }

            ImGui::EndDragDropTarget();
        }

        if (removeIndex >= 0 && removeIndex < static_cast<int>(clip.Frames.size()))
            clip.Frames.erase(clip.Frames.begin() + removeIndex);

        ImGui::EndChild();
    }

    void SpriteAtlasEditorPanel::AddSelectedGridCellsAsFrames(SpriteAtlas& atlas)
    {
        ImTextureID textureId{};
        glm::vec2 textureSize{ 0.0f };

        if (!GetTextureInfo(atlas, textureId, textureSize))
            return;

        m_GridTileSize.x = std::max(1, m_GridTileSize.x);
        m_GridTileSize.y = std::max(1, m_GridTileSize.y);
        m_Cols = std::max(1, m_Cols);
        m_Rows = std::max(1, m_Rows);

        const bool useCellSize = m_GridMode == GridMode::Cellsize;

        const int xCount = useCellSize
            ? std::max(1, static_cast<int>(textureSize.x) / m_GridTileSize.x)
            : m_Cols;

        const glm::vec2 cellSize = useCellSize
            ? glm::vec2(static_cast<float>(m_GridTileSize.x), static_cast<float>(m_GridTileSize.y))
            : glm::vec2(textureSize.x / static_cast<float>(m_Cols), textureSize.y / static_cast<float>(m_Rows));

        for (int cell : m_SelectedGridCells)
        {
            int x = cell % xCount;
            int y = cell / xCount;

            glm::vec2 pixelPos = {
                static_cast<float>(x) * cellSize.x,
                static_cast<float>(y) * cellSize.y
            };

            SpriteFrame frame = PixelToFrame(pixelPos, cellSize, textureSize);

            m_SelectedSprite = atlas.AddSpriteFrame(frame);
        }

        m_SelectedGridCells.clear();
        m_Mode = AtlasEditorMode::Select;
    }

    void SpriteAtlasEditorPanel::AddSelectedFrameToCurrentClip(SpriteAtlas& atlas)
    {
        if (!atlas.Exists(m_SelectedSprite))
            return;

        if (!atlas.IsValidClip(m_SelectedClip))
            return;

        atlas.GetClip(m_SelectedClip).Frames.push_back(m_SelectedSprite);
    }

    bool SpriteAtlasEditorPanel::GetTextureInfo(SpriteAtlas& atlas, ImTextureID& outTextureId, glm::vec2& outTextureSize) const
    {
        if (!atlas.GetTexture().IsValid())
            return false;

        auto textureAsset = atlas.GetTexture();
        if (!textureAsset)
            return false;

        auto texture = textureAsset->GetInstance();
        if (!texture)
            return false;

        outTextureId = texture->GetRendererID();
        outTextureSize = {
            static_cast<float>(texture->GetWidth()),
            static_cast<float>(texture->GetHeight())
        };

        return true;
    }

    int SpriteAtlasEditorPanel::HitTestFrame(SpriteAtlas& atlas, const glm::vec2& pixelPos) const
    {
        ImTextureID textureId{};
        glm::vec2 textureSize{ 0.0f };

        if (!GetTextureInfo(atlas, textureId, textureSize))
            return -1;

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

    glm::vec2 SpriteAtlasEditorPanel::UVToPixel(const SpriteFrame& frame, const glm::vec2& textureSize)
    {
        return {
            frame.UV.x * textureSize.x,
            (1.0f - frame.UV.y - frame.Size.y) * textureSize.y
        };
    }

    glm::vec2 SpriteAtlasEditorPanel::SizeToPixel(const SpriteFrame& frame, const glm::vec2& textureSize)
    {
        return {
            frame.Size.x * textureSize.x,
            frame.Size.y * textureSize.y
        };
    }

    SpriteFrame SpriteAtlasEditorPanel::PixelToFrame(
        const glm::vec2& pixelPos,
        const glm::vec2& pixelSize,
        const glm::vec2& textureSize)
    {
        SpriteFrame frame{};

        frame.UV = {
            pixelPos.x / textureSize.x,
            1.0f - ((pixelPos.y + pixelSize.y) / textureSize.y)
        };

        frame.Size = {
            pixelSize.x / textureSize.x,
            pixelSize.y / textureSize.y
        };
        return frame;
    }

    void SpriteAtlasEditorPanel::RestartPreview()
    {
        m_PreviewFrame = 0;
        m_PreviewTimer = 0.0f;
        m_PreviewPlaying = true;
    }

    void SpriteAtlasEditorPanel::UpdateAnimationPreview(SpriteAtlas& atlas)
    {
        if (!atlas.IsValidClip(m_SelectedClip))
            return;

        if (m_LastPreviewClip != m_SelectedClip)
        {
            m_LastPreviewClip = m_SelectedClip;
            RestartPreview();
        }

        SpriteAnimClip& clip = atlas.GetClip(m_SelectedClip);

        if (clip.Frames.empty())
        {
            m_PreviewFrame = 0;
            m_PreviewTimer = 0.0f;
            return;
        }

        m_PreviewFrame = std::clamp(
            m_PreviewFrame,
            0,
            static_cast<int>(clip.Frames.size()) - 1
        );

        if (!m_PreviewPlaying)
            return;

        int frameId = clip.Frames[m_PreviewFrame];

        if (!atlas.Exists(frameId))
            return;
        
        const SpriteFrame& frame = atlas.GetSpriteFrame(frameId);

        float frameTime = 1.0f / std::max(clip.FPS, 0.001f);

        m_PreviewTimer += ImGui::GetIO().DeltaTime * clip.Speed;

        if (m_PreviewTimer >= frameTime)
        {
            m_PreviewTimer -= frameTime;
            m_PreviewFrame = (m_PreviewFrame + 1) % clip.Frames.size();
        }
    }
}