#include "Panels/ContentBrowser.h"
#include <imgui.h>
#include <imgui_internal.h>

#include <Asset/TilemapAsset.h>
#include <Asset/SpriteAtlasAsset.h>
#include <Core/Application.h>

#include <algorithm>

using namespace BoonEditor;
namespace fs = std::filesystem;

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
    database.ForEachEntry([this](AssetHandle, const std::string& assetPath)
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
    // AssetDatabase now stores logical paths, e.g. "maps/tilemap.btm"
    // Do NOT make this relative to m_root again.
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

    constexpr float splitter = 4.0f;

    if (m_HierarchyWidth < 100.0f)
        m_HierarchyWidth = 100.0f;

    if (m_HierarchyWidth > avail.x * 0.9f)
        m_HierarchyWidth = avail.x * 0.9f;

    ImGui::BeginChild("##ContentBrowserRoot", avail, false);

    ImGui::BeginChild("##HierarchyPanel", ImVec2(m_HierarchyWidth, avail.y), true);
    DrawFolderTree(&m_RootFolder);
    ImGui::EndChild();

    ImGui::SameLine(0.0f, 1.0f);

    ImGui::Button("##Splitter", ImVec2(splitter, avail.y));

    const bool hovered = ImGui::IsItemHovered();
    const bool held = ImGui::IsItemActive() && ImGui::IsMouseDown(0);

    if (hovered || held)
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

    static bool dragging = false;

    if (held)
        dragging = true;
    else if (!ImGui::IsMouseDown(0))
        dragging = false;

    if (dragging)
    {
        const float mouseX = ImGui::GetIO().MousePos.x;
        const float windowX = ImGui::GetWindowPos().x;
        m_HierarchyWidth = mouseX - windowX;
    }

    ImGui::SameLine();

    ImGui::BeginChild("##ContentPanel", ImVec2(0, avail.y), true);

    if (!m_CurrentFolder)
        m_CurrentFolder = &m_RootFolder;

    DrawContentArea(m_CurrentFolder);

    ImGui::EndChild();

    ImGui::EndChild();
}

void ContentBrowser::DrawFolderTree(FolderNode* node)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 10.0f);

    if (node == &m_RootFolder)
    {
        const bool selected = m_CurrentFolder == &m_RootFolder;

        ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_SpanFullWidth |
            ImGuiTreeNodeFlags_DefaultOpen |
            ImGuiTreeNodeFlags_FramePadding;

        if (selected)
            flags |= ImGuiTreeNodeFlags_Selected;

        const bool open = ImGui::TreeNodeEx("Content", flags, "Content");

        if (ImGui::IsItemClicked())
            m_CurrentFolder = &m_RootFolder;

        if (open)
        {
            for (auto& [name, child] : node->children)
                DrawFolderTree(child);

            ImGui::TreePop();
        }

        ImGui::PopStyleVar(3);
        return;
    }

    const bool isSelected = node == m_CurrentFolder;

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_SpanFullWidth |
        ImGuiTreeNodeFlags_FramePadding;

    if (node->children.empty())
        flags |= ImGuiTreeNodeFlags_Leaf;

    if (isSelected)
        flags |= ImGuiTreeNodeFlags_Selected;

    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.28f, 0.28f, 0.38f, 0.35f));
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.20f, 0.20f, 0.30f, 0.45f));

    const bool open = ImGui::TreeNodeEx(
        node->name.c_str(),
        flags,
        "%s",
        node->name.c_str());

    ImGui::PopStyleColor(2);

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

    const float panelWidth = ImGui::GetContentRegionAvail().x;

    constexpr float topBarHeight = 28.0f;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2));

    ImGui::BeginChild(
        "CB_TopBar",
        ImVec2(0, topBarHeight),
        true,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    const float totalWidth = ImGui::GetContentRegionAvail().x;
    const float searchWidth = totalWidth * (1.0f / 3.0f);
    const float sliderWidth = 60.0f;

    const float widgetHeight = ImGui::GetFrameHeight();
    const float yOffset = (topBarHeight - widgetHeight) * 0.5f;

    ImGui::SetCursorPosY(yOffset);

    ImGui::TextUnformatted("search");
    ImGui::SameLine();

    ImGui::SetNextItemWidth(searchWidth);
    ImGui::InputText("##Search", m_SearchBuffer, sizeof(m_SearchBuffer));

    ImGui::SameLine();
    ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(), totalWidth - sliderWidth));

    static int desiredColumns = 16;
    desiredColumns = std::clamp(desiredColumns, 2, 20);

    ImGui::SetNextItemWidth(sliderWidth);
    ImGui::SliderInt("##Cols", &desiredColumns, 2, 20, "");

    ImGui::EndChild();

    ImGui::PopStyleVar(2);

    ImGui::BeginChild("CB_Content", ImVec2(0, 0), false);

    constexpr float padding = 8.0f;

    float iconSize = (panelWidth / static_cast<float>(desiredColumns)) - padding;
    iconSize = std::clamp(iconSize, 32.0f, 256.0f);

    const float colWidth = iconSize + padding;

    int columns = desiredColumns;
    while (columns > 2 && columns * colWidth > panelWidth)
        --columns;

    columns = std::max(columns, 1);

    ImGuiTableFlags tableFlags =
        ImGuiTableFlags_NoBordersInBody |
        ImGuiTableFlags_NoPadOuterX |
        ImGuiTableFlags_SizingFixedFit |
        ImGuiTableFlags_NoHostExtendX;

    if (ImGui::BeginTable("CB_Grid", columns, tableFlags))
    {
        for (int col = 0; col < columns; ++col)
            ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed, colWidth);

        std::vector<std::string> displayAssets;

        if (m_SearchBuffer[0] != '\0')
            CollectAssetsRecursive(folder, displayAssets);
        else
            displayAssets = folder->assets;

        for (const std::string& assetPath : displayAssets)
        {
            const std::string filename = fs::path(assetPath).filename().string();

            ImGui::PushID(assetPath.c_str());
            ImGui::TableNextColumn();

            const float cellStartX = ImGui::GetCursorPosX();
            const float cellWidth = ImGui::GetColumnWidth();

            // IMPORTANT:
            // Use logical path. Do NOT prepend m_root.
            AssetHandle h = AssetDatabase::Get().GetHandle(assetPath);

            const bool selected = m_pSelectedAsset && m_pSelectedAsset->Get() == h;

            const ImVec4 bgHovered = ImVec4(0.15f, 0.25f, 0.35f, 0.35f);
            const ImVec4 bgActive = ImVec4(0.15f, 0.25f, 0.35f, 0.50f);
            const ImVec4 bg = selected ? bgActive : ImVec4(0, 0, 0, 0);

            ImGui::PushStyleColor(ImGuiCol_Button, bg);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bgHovered);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, bgActive);

            AssetRef<Texture2DAsset> thumbnail = AssetDatabase::Get().GetThumbnail(h);

            const float iconX = cellStartX + (cellWidth - iconSize) * 0.5f;
            ImGui::SetCursorPosX(iconX);

            bool pressed = false;

            if (thumbnail.IsValid())
            {
                pressed = ImGui::ImageButton(
                    "##thumb",
                    thumbnail->GetInstance()->GetRendererID(),
                    ImVec2(iconSize, iconSize));
            }
            else
            {
                pressed = ImGui::Button("##thumb", ImVec2(iconSize, iconSize));
            }

            ImGui::PopStyleColor(3);

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
                    ImGui::TextUnformatted(filename.c_str());
                }

                ImGui::EndDragDropSource();
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", filename.c_str());

            const float wrapWidth = iconSize;
            const float maxTextHeight = ImGui::GetTextLineHeight() * 2.0f;

            const float labelX = cellStartX + (cellWidth - wrapWidth) * 0.5f;
            ImGui::SetCursorPosX(labelX);

            const ImVec2 labelStart = ImGui::GetCursorScreenPos();

            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + wrapWidth);
            ImGui::TextUnformatted(filename.c_str());
            ImGui::PopTextWrapPos();

            const ImVec2 labelEnd = ImGui::GetCursorScreenPos();
            const float usedTextHeight = labelEnd.y - labelStart.y;

            if (usedTextHeight > maxTextHeight)
            {
                ImGui::SetCursorScreenPos(ImVec2(
                    labelEnd.x,
                    labelStart.y + maxTextHeight));
            }

            ImGui::Dummy(ImVec2(cellWidth, 4.0f));

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
        ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
    {
        if (ImGui::MenuItem("New folder"))
        {
            fs::create_directory(fs::path(folder->fullPath) / "new_folder");
            BuildFolderTree();
        }

        if (ImGui::MenuItem("New sprite atlas"))
        {
            ServiceLocator::Get<AssetImporterRegistry>()
                .Export<SpriteAtlasAsset>(
                    fs::path(folder->fullPath) / "new_sprite.bsa",
                    0u);

            BuildFolderTree();
        }

        if (ImGui::MenuItem("New tilemap"))
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