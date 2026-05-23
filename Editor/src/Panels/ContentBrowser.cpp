#include "Panels/ContentBrowser.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <UI/IconsFontAwesome7.h>

#include <Asset/TilemapAsset.h>
#include <Asset/SpriteAtlasAsset.h>
#include <Core/Application.h>

#include <algorithm>
#include <cstdio>

using namespace BoonEditor;
namespace fs = std::filesystem;

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

    const char* GetAssetIcon(const std::string& path)
    {
        const std::string ext = fs::path(path).extension().string();

        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")
            return ICON_FA_IMAGE;

        if (ext == ".bsa")
            return ICON_FA_IMAGES;

        if (ext == ".btm")
            return ICON_FA_TABLE_CELLS;

        return ICON_FA_FILE;
    }

    bool IconButton(
        const char* icon,
        const char* tooltip,
        ImVec2 size = ImVec2(28.0f, 28.0f))
    {
        bool pressed = ImGui::Button(icon, size);

        if (ImGui::IsItemHovered() && tooltip)
            ImGui::SetTooltip("%s", tooltip);

        return pressed;
    }

    void DrawPanelTitle(const char* icon, const char* title)
    {
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s  %s", icon, title);
    }

    std::string EllipsizeText(const std::string& text, float maxWidth)
    {
        if (ImGui::CalcTextSize(text.c_str()).x <= maxWidth)
            return text;

        std::string result = text;

        while (!result.empty())
        {
            result.pop_back();

            std::string candidate = result + "..";

            if (ImGui::CalcTextSize(candidate.c_str()).x <= maxWidth)
                return candidate;
        }

        return "..";
    }
}

ContentBrowser::ContentBrowser(
    const std::string& name,
    EditorContext* pContext,
    AssetContext* pAsset)
    : EditorPanel(name, pContext)
    , m_RootFolder("Content", "")
    , m_pSelectedAsset(pAsset)
{
    memset(m_SearchBuffer, 0, sizeof(m_SearchBuffer));

    m_CurrentFolder = &m_RootFolder;

    BuildFolderTree();
}

void ContentBrowser::Update()
{
    auto& db = AssetDatabase::Get();

    if (db.IsDirty())
    {
        BuildFolderTree();
        db.ClearDirty();
    }
}

void ContentBrowser::BuildFolderTree()
{
    m_RootFolder.children.clear();
    m_RootFolder.assets.clear();

    const RuntimeConfig& config = Application::Get().GetDescriptor();

    m_root = config.AssetsRoot;
    m_RootFolder.name = "Content";
    m_RootFolder.fullPath = m_root.string();

    m_CurrentFolder = &m_RootFolder;

    if (!fs::exists(m_root))
        fs::create_directories(m_root);

    BuildFoldersFromDisk(m_root.string(), &m_RootFolder);

    auto& database = AssetDatabase::Get();

    database.ForEachEntry(
        [this](AssetHandle, const std::string& assetPath)
        {
            AddAssetToTree(assetPath);
        });
}

void ContentBrowser::BuildFoldersFromDisk(const std::string& path, FolderNode* parent)
{
    if (!fs::exists(path))
        return;

    for (const auto& entry : fs::directory_iterator(path))
    {
        if (!entry.is_directory())
            continue;

        const fs::path fullPath = entry.path();
        const std::string name = fullPath.filename().string();

        FolderNode* node = new FolderNode(name, fullPath.string());
        parent->children[name] = node;

        BuildFoldersFromDisk(fullPath.string(), node);
    }
}

void ContentBrowser::AddAssetToTree(const std::string& path)
{
    fs::path p = fs::path(path).lexically_normal();

    if (p.empty())
        return;

    FolderNode* node = &m_RootFolder;

    const fs::path parentPath = p.parent_path();

    for (const auto& part : parentPath)
    {
        const std::string name = part.string();

        if (name.empty())
            continue;

        auto it = node->children.find(name);

        if (it == node->children.end())
            return;

        node = it->second;
    }

    const std::string logicalPath = p.generic_string();

    if (std::find(node->assets.begin(), node->assets.end(), logicalPath) == node->assets.end())
        node->assets.push_back(logicalPath);
}

void ContentBrowser::CollectAssetsRecursive(FolderNode* node, std::vector<std::string>& out)
{
    for (const auto& asset : node->assets)
        out.push_back(asset);

    for (auto& [name, child] : node->children)
        CollectAssetsRecursive(child, out);
}

void ContentBrowser::OnRenderUI()
{
    ImVec2 avail = ImGui::GetContentRegionAvail();

    if (avail.x <= 0.0f || avail.y <= 0.0f)
        return;

    constexpr float splitterWidth = 5.0f;
    constexpr float minHierarchyWidth = 130.0f;

    m_HierarchyWidth = std::max(m_HierarchyWidth, minHierarchyWidth);
    m_HierarchyWidth = std::min(m_HierarchyWidth, avail.x * 0.75f);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 6.0f));

    ImGui::BeginChild("##ContentBrowserRoot", avail, false);

    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, ImGui::GetStyle().WindowRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));

    ImGui::BeginChild("##HierarchyPanel", ImVec2(m_HierarchyWidth, avail.y), true);

    DrawPanelTitle(ICON_FA_FOLDER_TREE, "Folders");
    ImGui::Separator();

    DrawFolderTree(&m_RootFolder);

    ImGui::EndChild();

    ImGui::PopStyleVar(2);

    ImGui::SameLine(0.0f, 4.0f);

    ImGui::InvisibleButton("##ContentBrowserSplitter", ImVec2(splitterWidth, avail.y));

    const bool hovered = ImGui::IsItemHovered();
    const bool held = ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left);

    if (hovered || held)
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 splitMin = ImGui::GetItemRectMin();
    ImVec2 splitMax = ImGui::GetItemRectMax();

    drawList->AddRectFilled(
        splitMin,
        splitMax,
        hovered || held ? Accent(0.75f) : Border(),
        3.0f);

    static bool dragging = false;

    if (held)
        dragging = true;
    else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
        dragging = false;

    if (dragging)
    {
        const float mouseX = ImGui::GetIO().MousePos.x;
        const float windowX = ImGui::GetWindowPos().x;

        m_HierarchyWidth = mouseX - windowX;
        m_HierarchyWidth = std::clamp(m_HierarchyWidth, minHierarchyWidth, avail.x * 0.75f);
    }

    ImGui::SameLine(0.0f, 4.0f);

    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, ImGui::GetStyle().WindowRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));

    ImGui::BeginChild("##ContentPanel", ImVec2(0.0f, avail.y), true);

    if (!m_CurrentFolder)
        m_CurrentFolder = &m_RootFolder;

    DrawContentArea(m_CurrentFolder);

    ImGui::EndChild();

    ImGui::PopStyleVar(2);

    ImGui::EndChild();

    ImGui::PopStyleVar(2);
}

void ContentBrowser::DrawFolderTree(FolderNode* node)
{
    if (!node)
        return;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 2.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 14.0f);

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_SpanFullWidth |
        ImGuiTreeNodeFlags_FramePadding;

    if (node == &m_RootFolder)
        flags |= ImGuiTreeNodeFlags_DefaultOpen;

    if (node->children.empty())
        flags |= ImGuiTreeNodeFlags_Leaf;

    if (node == m_CurrentFolder)
        flags |= ImGuiTreeNodeFlags_Selected;

    ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyleColorVec4(ImGuiCol_Header));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));

    const char* icon =
        node == &m_RootFolder
        ? ICON_FA_FOLDER_TREE
        : ICON_FA_FOLDER;

    const bool open = ImGui::TreeNodeEx(
        node == &m_RootFolder ? "Content" : node->name.c_str(),
        flags,
        "%s  %s",
        icon,
        node == &m_RootFolder ? "Content" : node->name.c_str());

    ImGui::PopStyleColor(3);

    if (ImGui::IsItemClicked())
        m_CurrentFolder = node;

    if (open)
    {
        for (auto& [name, child] : node->children)
            DrawFolderTree(child);

        ImGui::TreePop();
    }

    ImGui::PopStyleVar(3);
}

void ContentBrowser::DrawContentArea(FolderNode* folder)
{
    if (!folder)
        folder = &m_RootFolder;

    static int desiredColumns = 10;
    desiredColumns = std::clamp(desiredColumns, 2, 20);

    const float panelWidth = ImGui::GetContentRegionAvail().x;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 5.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 4.0f));

    ImGui::BeginChild(
        "##ContentBrowserTopBar",
        ImVec2(0.0f, 38.0f),
        false,
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse);

    DrawPanelTitle(ICON_FA_FOLDER_OPEN, "Content Browser");

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::Text("| %s", folder->name.c_str());
    ImGui::PopStyleColor();

    ImGui::SameLine();

    const float rightWidth = 340.0f;
    const float cursorX = ImGui::GetCursorPosX();
    const float availableX = ImGui::GetContentRegionAvail().x;

    if (availableX > rightWidth)
        ImGui::SetCursorPosX(cursorX + availableX - rightWidth);

    ImGui::SetNextItemWidth(220.0f);

    ImGui::InputTextWithHint(
        "##ContentSearch",
        ICON_FA_MAGNIFYING_GLASS " Search assets...",
        m_SearchBuffer,
        sizeof(m_SearchBuffer));

    ImGui::SameLine();

    ImGui::SetNextItemWidth(86.0f);

    ImGui::SliderInt(
        "##ContentColumns",
        &desiredColumns,
        2,
        20,
        ICON_FA_GRIP " %d");

    ImGui::EndChild();

    ImGui::PopStyleVar(2);

    ImGui::Separator();

    ImGui::BeginChild("##ContentBrowserAssets", ImVec2(0.0f, 0.0f), false);

    constexpr float padding = 12.0f;

    float iconSize = (panelWidth / static_cast<float>(desiredColumns)) - padding;
    iconSize = std::clamp(iconSize, 42.0f, 180.0f);

    const float colWidth = iconSize + padding;

    int columns = desiredColumns;

    while (columns > 1 && columns * colWidth > panelWidth)
        --columns;

    columns = std::max(columns, 1);

    std::vector<std::string> displayAssets;

    if (m_SearchBuffer[0] != '\0')
        CollectAssetsRecursive(folder, displayAssets);
    else
        displayAssets = folder->assets;

    if (displayAssets.empty())
    {
        ImGui::Dummy(ImVec2(0.0f, 48.0f));

        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));

        const char* emptyText = ICON_FA_BOX_OPEN "  Empty folder";
        const ImVec2 textSize = ImGui::CalcTextSize(emptyText);

        ImGui::SetCursorPosX(std::max(0.0f, (ImGui::GetContentRegionAvail().x - textSize.x) * 0.5f));
        ImGui::TextUnformatted(emptyText);

        ImGui::PopStyleColor();
    }

    ImGuiTableFlags tableFlags =
        ImGuiTableFlags_NoBordersInBody |
        ImGuiTableFlags_NoPadOuterX |
        ImGuiTableFlags_SizingFixedFit |
        ImGuiTableFlags_NoHostExtendX;

    if (!displayAssets.empty() && ImGui::BeginTable("##ContentGrid", columns, tableFlags))
    {
        for (int col = 0; col < columns; ++col)
            ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed, colWidth);

        for (const std::string& assetPath : displayAssets)
        {
            const std::string filename = fs::path(assetPath).filename().string();

            if (m_SearchBuffer[0] != '\0')
            {
                std::string lowerFile = filename;
                std::string lowerSearch = m_SearchBuffer;

                std::transform(lowerFile.begin(), lowerFile.end(), lowerFile.begin(), ::tolower);
                std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);

                if (lowerFile.find(lowerSearch) == std::string::npos)
                    continue;
            }

            ImGui::PushID(assetPath.c_str());
            ImGui::TableNextColumn();

            const float cellStartX = ImGui::GetCursorPosX();
            const float cellWidth = ImGui::GetColumnWidth();

            AssetHandle h = AssetDatabase::Get().GetHandle(assetPath);
            const bool selected = m_pSelectedAsset && m_pSelectedAsset->Get() == h;

            AssetRef<Texture2DAsset> thumbnail = AssetDatabase::Get().GetThumbnail(h);

            const float cardWidth = iconSize;
            const float cardHeight = iconSize + ImGui::GetTextLineHeight() * 2.4f;

            const float cardX = cellStartX + (cellWidth - cardWidth) * 0.5f;
            ImGui::SetCursorPosX(cardX);

            ImVec2 cardPos = ImGui::GetCursorScreenPos();

            ImGui::InvisibleButton("##AssetCard", ImVec2(cardWidth, cardHeight));

            const bool hovered = ImGui::IsItemHovered();
            const bool pressed = ImGui::IsItemClicked();

            if (pressed && m_pSelectedAsset)
                m_pSelectedAsset->Set(h);

            if (ImGui::BeginDragDropSource())
            {
                ImGui::SetDragDropPayload("ASSET_HANDLE", &h, sizeof(AssetHandle));

                if (thumbnail.IsValid())
                {
                    ImGui::Image(
                        thumbnail->GetInstance()->GetRendererID(),
                        ImVec2(iconSize * 0.5f, iconSize * 0.5f));
                }
                else
                {
                    ImGui::Text("%s  %s", GetAssetIcon(assetPath), filename.c_str());
                }

                ImGui::EndDragDropSource();
            }

            if (hovered)
                ImGui::SetTooltip("%s", filename.c_str());

            ImDrawList* drawList = ImGui::GetWindowDrawList();

            ImVec2 cardMax(
                cardPos.x + cardWidth,
                cardPos.y + cardHeight);

            ImU32 bg =
                selected ? Accent(0.20f) :
                hovered ? SurfaceHover() :
                Surface();

            ImU32 outline =
                selected ? Accent(0.95f) :
                hovered ? Accent(0.55f) :
                Border();

            //drawList->AddRectFilled(cardPos, cardMax, bg, 10.0f);

            if (selected || hovered)
                drawList->AddRect(cardPos, cardMax, outline, 10.0f, 0, selected ? 2.0f : 1.0f);

            const float thumbPadding = 10.0f;
            ImVec2 thumbMin(
                cardPos.x + thumbPadding,
                cardPos.y + thumbPadding);

            ImVec2 thumbMax(
                cardPos.x + cardWidth - thumbPadding,
                cardPos.y + cardWidth - thumbPadding);

            //drawList->AddRectFilled(
            //    thumbMin,
            //    thumbMax,
            //    ImGui::GetColorU32(ImGuiCol_WindowBg),
            //    8.0f);

            if (thumbnail.IsValid())
            {
                drawList->AddImage(
                    thumbnail->GetInstance()->GetRendererID(),
                    thumbMin,
                    thumbMax);
            }
            else
            {
                const char* icon = GetAssetIcon(assetPath);
                ImVec2 iconTextSize = ImGui::CalcTextSize(icon);

                drawList->AddText(
                    ImVec2(
                        thumbMin.x + (thumbMax.x - thumbMin.x - iconTextSize.x) * 0.5f,
                        thumbMin.y + (thumbMax.y - thumbMin.y - iconTextSize.y) * 0.5f),
                    selected ? Accent(1.0f) : TextMuted(),
                    icon);
            }

            //drawList->AddRect(thumbMin, thumbMax, Border(), 8.0f);

            const float labelWidth = cardWidth - 12.0f;
            const float labelX = cardPos.x + 6.0f;
            const float labelY = thumbMax.y + 7.0f;

            ImGui::SetCursorScreenPos(ImVec2(labelX, labelY));

            std::string displayName = EllipsizeText(filename, labelWidth);

            drawList->AddText(
                ImVec2(labelX, labelY),
                selected ? Col(ImGuiCol_Text) : TextMuted(),
                displayName.c_str());

            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
        ImGui::IsWindowHovered() &&
        !ImGui::IsAnyItemHovered() &&
        m_pSelectedAsset)
    {
        m_pSelectedAsset->Set(0u);
    }

    if (ImGui::BeginPopupContextWindow(
        "addasset",
        ImGuiPopupFlags_MouseButtonRight |
        ImGuiPopupFlags_NoOpenOverItems))
    {
        if (ImGui::MenuItem(ICON_FA_FOLDER_PLUS "  New Folder"))
        {
            fs::create_directory(fs::path(folder->fullPath) / "new_folder");
            BuildFolderTree();
        }

        ImGui::Separator();

        if (ImGui::MenuItem(ICON_FA_IMAGES "  New Sprite Atlas"))
        {
            ServiceLocator::Get<AssetImporterRegistry>()
                .Export<SpriteAtlasAsset>(
                    fs::path(folder->fullPath) / "new_sprite.bsa",
                    0u);

            BuildFolderTree();
        }

        if (ImGui::MenuItem(ICON_FA_TABLE_CELLS "  New Tilemap"))
        {
            ServiceLocator::Get<AssetImporterRegistry>()
                .Export<TilemapAsset>(
                    fs::path(folder->fullPath) / "new_tilemap.btm",
                    0u);

            BuildFolderTree();
        }

        ImGui::EndPopup();
    }

    ImGui::EndChild();
}