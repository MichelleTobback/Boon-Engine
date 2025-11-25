#include "Panels/ContentBrowser.h"
#include <imgui.h>
#include <imgui_internal.h>

using namespace BoonEditor;
namespace fs = std::filesystem;

ContentBrowser::ContentBrowser(const std::string& name, DragDropRouter* pRouter, AssetContext* pContext)
    : EditorPanel(name, pRouter), m_RootFolder("Content", ""), m_pSelectedAsset(pContext)
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

//
// ────────────────────────────────────────────────────────────────
//   BUILD TREE
// ────────────────────────────────────────────────────────────────
//

void ContentBrowser::BuildFolderTree()
{
    m_RootFolder.children.clear();
    m_RootFolder.assets.clear();

    // Asset root must match AssetDirectoryScanner's root
    std::string root = BoonEditor::AssetDatabase::Get().GetAssetRoot();
    m_RootFolder.fullPath = root;

    // 1. Build real folder structure
    BuildFoldersFromDisk(root, &m_RootFolder);

    // 2. Insert only registered assets
    auto& database = AssetDatabase::Get();
    database.ForEachEntry([this](AssetHandle handle, const std::string& assetPath)
        {
            AddAssetToTree(assetPath);
        });
}

void ContentBrowser::BuildFoldersFromDisk(const std::string& path, FolderNode* parent)
{
    for (auto& entry : fs::directory_iterator(path))
    {
        if (entry.is_directory())
        {
            std::string name = entry.path().filename().string();
            std::string full = entry.path().string();

            FolderNode* node = new FolderNode(name, full);
            parent->children[name] = node;

            BuildFoldersFromDisk(full, node);
        }
    }
}

void ContentBrowser::AddAssetToTree(const std::string& path)
{
    std::string cleanPath = path;

    // Normalize slashes
    std::replace(cleanPath.begin(), cleanPath.end(), '\\', '/');

    // If path starts with "Asset/" → remove it
    constexpr const char* prefix = "Assets/";
    if (cleanPath.rfind(prefix, 0) == 0)
        cleanPath = cleanPath.substr(strlen(prefix));

    fs::path p(cleanPath);

    FolderNode* node = &m_RootFolder;

    // Walk through parent folders
    fs::path parentPath = p.parent_path();

    for (auto& part : parentPath)
    {
        std::string name = part.string();
        if (name.empty())
            continue;

        auto it = node->children.find(name);
        if (it == node->children.end())
        {
            // Disk folder not found → ignore asset (don't create fake folders)
            return;
        }

        node = it->second;
    }

    node->assets.push_back(cleanPath);
}

void ContentBrowser::CollectAssetsRecursive(FolderNode* node, std::vector<std::string>& out)
{
    // Add assets in this folder
    for (auto& a : node->assets)
        out.push_back(a);

    // Recurse into subfolders
    for (auto& [name, child] : node->children)
        CollectAssetsRecursive(child, out);
}

//
// ────────────────────────────────────────────────────────────────
//   UI RENDERING
// ────────────────────────────────────────────────────────────────
//

void ContentBrowser::OnRenderUI()
{
    ImVec2 avail = ImGui::GetContentRegionAvail();

    float splitter = 4.0f;

    if (m_HierarchyWidth < 100.f) m_HierarchyWidth = 100.f;
    if (m_HierarchyWidth > avail.x * 0.9f) m_HierarchyWidth = avail.x * 0.9f;

    ImGui::BeginChild("##ContentBrowserRoot", avail, false);

    //
    // LEFT: FOLDER TREE
    //
    ImGui::BeginChild("##HierarchyPanel", ImVec2(m_HierarchyWidth, avail.y), true);
    DrawFolderTree(&m_RootFolder);
    ImGui::EndChild();

    //
    // SPLITTER DRAG
    //
    ImGui::SameLine(0.0f, 1.0f);
    ImGui::Button("##Splitter", ImVec2(splitter, avail.y));

    bool hovered = ImGui::IsItemHovered();
    bool held = ImGui::IsItemActive() && ImGui::IsMouseDown(0);

    if (hovered || held)
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

    static bool dragging = false;

    if (held) dragging = true;
    else if (!ImGui::IsMouseDown(0)) dragging = false;

    if (dragging)
    {
        float mouseX = ImGui::GetIO().MousePos.x;
        float windowX = ImGui::GetWindowPos().x;
        m_HierarchyWidth = mouseX - windowX;
    }

    //
    // RIGHT: CONTENT GRID
    //
    ImGui::SameLine();
    ImGui::BeginChild("##ContentPanel", ImVec2(0, avail.y), true);

    //ImGui::InputText("Search", m_SearchBuffer, sizeof(m_SearchBuffer));
    DrawContentArea(m_CurrentFolder);

    ImGui::EndChild();
    ImGui::EndChild();
}

//
// ────────────────────────────────────────────────────────────────
//   FOLDER TREE
// ────────────────────────────────────────────────────────────────
//

void ContentBrowser::DrawFolderTree(FolderNode* node)
{
    // Make the tree much more compact
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));  // smaller height
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 1));   // reduce vertical spacing
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 10.0f);        // tighter indentation

    // Draw root
    if (node == &m_RootFolder)
    {
        ImGui::TextUnformatted("Content");
    }

    for (auto& [name, child] : node->children)
    {
        bool isSelected = (child == m_CurrentFolder);

        ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_SpanFullWidth |
            ImGuiTreeNodeFlags_FramePadding;

        if (child->children.empty())
            flags |= ImGuiTreeNodeFlags_Leaf;

        if (isSelected)
            flags |= ImGuiTreeNodeFlags_Selected;

        // Light hover & selected background (theme-friendly)
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.28f, 0.28f, 0.38f, 0.35f));
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.20f, 0.20f, 0.30f, 0.45f));

        // We don't use Unicode icons → pure text
        const char* label = name.c_str();

        bool open = ImGui::TreeNodeEx(
            child->name.c_str(),
            flags,
            "%s",
            label
        );

        ImGui::PopStyleColor(2);

        if (ImGui::IsItemClicked())
            m_CurrentFolder = child;

        if (open)
        {
            DrawFolderTree(child);
            ImGui::TreePop();
        }
    }

    ImGui::PopStyleVar(3);
}

//
// ────────────────────────────────────────────────────────────────
//   CONTENT AREA
// ────────────────────────────────────────────────────────────────
//

void ContentBrowser::DrawContentArea(FolderNode* folder)
{
    float panelWidth = ImGui::GetContentRegionAvail().x;

    //
    // ───────────────────────────────────────────────
    //  FIXED TOP BAR (Search + Column Slider)
    // ───────────────────────────────────────────────
    //
    const float topBarHeight = 28.0f;

    // Smaller padding so controls fit nicely
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2));

    ImGui::BeginChild("CB_TopBar", ImVec2(0, topBarHeight), true,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    float totalWidth = ImGui::GetContentRegionAvail().x;

    // Calculate 1/3 width for search bar
    float searchWidth = totalWidth * (1.0f / 3.0f);

    // Slider width stays constant
    float sliderWidth = 60.0f;

    // Center controls vertically
    float widgetHeight = ImGui::GetFrameHeight();
    float yOffset = (topBarHeight - widgetHeight) * 0.5f;
    ImGui::SetCursorPosY(yOffset);

    //
    // ─────────────────────────
    //  Search bar (1/3 width)
    // ─────────────────────────
    //
    ImGui::TextUnformatted("search");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(searchWidth);
    ImGui::InputText("##Search", m_SearchBuffer, sizeof(m_SearchBuffer));

    //
    // ─────────────────────────
    //  Right-aligned slider
    // ─────────────────────────
    //

    // Move to the right edge
    float sliderX = totalWidth - sliderWidth;
    ImGui::SameLine();
    ImGui::SetCursorPosX(sliderX);

    static int desiredColumns = 16;
    desiredColumns = std::clamp(desiredColumns, 2, 20);

    ImGui::SetNextItemWidth(sliderWidth);
    ImGui::SliderInt("##Cols", &desiredColumns, 2, 20, "");

    ImGui::EndChild();

    ImGui::PopStyleVar(2);

    //
    // ───────────────────────────────────────────────
    //  SCROLLABLE CONTENT GRID USING IMGUI TABLES
    // ───────────────────────────────────────────────
    //

    ImGui::BeginChild("CB_Content", ImVec2(0, 0), false);

    // padding around each cell
    float padding = 8.0f;

    // Compute icon size from slider
    float iconSize = (panelWidth / desiredColumns) - padding;
    iconSize = std::clamp(iconSize, 32.0f, 256.0f);

    // Column width = icon + padding
    float colWidth = iconSize + padding;

    int colums = desiredColumns;
    // If panel too small, reduce number of columns
    while (colums > 2 && colums * colWidth > panelWidth)
    {
        colums--;
    }

    ImGuiTableFlags tableFlags =
        ImGuiTableFlags_NoBordersInBody |
        ImGuiTableFlags_NoPadOuterX |
        ImGuiTableFlags_SizingFixedFit |    // ← FIX: tightly packed
        ImGuiTableFlags_NoHostExtendX;

    if (ImGui::BeginTable("CB_Grid", colums, tableFlags))
    {
        for (int col = 0; col < colums; col++)
            ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed, colWidth);

        std::vector<std::string> displayAssets;

        if (m_SearchBuffer[0] != '\0')
        {
            // SEARCH MODE → RECURSIVE
            CollectAssetsRecursive(folder, displayAssets);
        }
        else
        {
            // NORMAL MODE → ONLY DIRECT CHILDREN
            displayAssets = folder->assets;
        }

        for (const std::string& assetPath : displayAssets)
        {
            //
            // Search filter (recursive when search bar is not empty)
            //
            std::string filename = std::filesystem::path(assetPath).filename().string();

            ImGui::PushID(assetPath.c_str());
            ImGui::TableNextColumn();

            //
            // ICON (centered)
            //
            float cellWidth = ImGui::GetColumnWidth();
            float iconX = (cellWidth - iconSize) * 0.5f;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + iconX);

            AssetHandle h = AssetDatabase::Get().GetHandle("Assets/" + assetPath);
            bool selected = m_pSelectedAsset->Get() == h;

            ImVec4 bgHovered = ImVec4(0.15f, 0.25f, 0.35f, 0.35f);
            ImVec4 bgActive = ImVec4(0.15f, 0.25f, 0.35f, 0.50f);
            ImVec4 bg = selected ? bgActive : ImVec4(0, 0, 0, 0);

            ImGui::PushStyleColor(ImGuiCol_Button, bg);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bgHovered);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, bgActive);

            bool pressed = false;
            AssetRef<Texture2DAsset> thumbnail = AssetDatabase::Get().GetThumbnail(h);
            if (thumbnail.IsValid())
                pressed = ImGui::ImageButton("##thumb", thumbnail->GetInstance()->GetRendererID(), ImVec2(iconSize, iconSize));
            else
                pressed = ImGui::Button("##thumb", ImVec2(iconSize, iconSize));

            ImGui::PopStyleColor(3);

            if (pressed)
            {
                m_pSelectedAsset->Set(h);
            }

            // Drag & drop
            if (ImGui::BeginDragDropSource())
            {
                ImGui::SetDragDropPayload("ASSET_HANDLE", &h, sizeof(AssetHandle));

                if (thumbnail.IsValid())
                    ImGui::Image(thumbnail->GetInstance()->GetRendererID(), ImVec2(iconSize * 0.5f, iconSize * 0.5f));
                else
                    ImGui::Text("%s", filename.c_str());

                ImGui::EndDragDropSource();
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", filename.c_str());

            //
            // 2-LINE CLAMPED, CENTERED LABEL
            //

            //ImGui::Dummy(ImVec2(0, 1)); // spacing

            float wrapWidth = iconSize;
            float maxTextHeight = ImGui::GetTextLineHeight() * 2;

            // Center the text-block itself (not the raw text width)
            float textBlockX = (cellWidth - wrapWidth) * 0.5f;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textBlockX);

            // Begin a constrained region that forces wrapping
            ImGui::BeginChild("##label",
                ImVec2(wrapWidth, maxTextHeight),
                false,
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse);

            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + wrapWidth);
            ImGui::TextUnformatted(filename.c_str());
            ImGui::PopTextWrapPos();

            ImGui::EndChild();

            ImGui::PopID();
        }

        ImGui::EndTable();

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            m_pSelectedAsset->Set(0u);
        }
    }

    ImGui::EndChild();

}





