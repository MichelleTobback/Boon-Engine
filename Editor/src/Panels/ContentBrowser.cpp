#include "Panels/ContentBrowser.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <UI/IconsFontAwesome7.h>

#include <Asset/TilemapAsset.h>
#include <Asset/SpriteAtlasAsset.h>
#include <Asset/MaterialAsset.h>
#include <Asset/TilemapAsset.h>
#include <Core/Application.h>
#include <Core/EditorContext.h>
#include <Command/EditorCommandQueue.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <functional>
#include <string>
#include <system_error>
#include <vector>

using namespace BoonEditor;
namespace fs = std::filesystem;

namespace
{
    struct FolderDragPayload
    {
        char Path[512]{};
    };

    constexpr const char* FolderPayloadName = "CONTENT_BROWSER_FOLDER";

    enum class PendingCreateType
    {
        None,
        Folder,
        SpriteAtlas,
        Tilemap,
        Material
    };

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

    std::string NormalizePathString(const fs::path& path)
    {
        return path.lexically_normal().generic_string();
    }

    bool IsSamePath(const fs::path& a, const fs::path& b)
    {
        std::error_code ecA;
        std::error_code ecB;

        fs::path absA = fs::weakly_canonical(a, ecA);
        fs::path absB = fs::weakly_canonical(b, ecB);

        if (ecA)
            absA = a.lexically_normal();

        if (ecB)
            absB = b.lexically_normal();

        return NormalizePathString(absA) == NormalizePathString(absB);
    }

    bool IsPathInside(const fs::path& child, const fs::path& parent)
    {
        std::error_code ec;

        fs::path rel = fs::relative(child.lexically_normal(), parent.lexically_normal(), ec);

        if (ec || rel.empty())
            return false;

        const std::string relString = rel.generic_string();

        return relString != "." && !relString.starts_with("..");
    }

    bool CanMoveFolderTo(const fs::path& sourceFolder, const fs::path& destinationFolder)
    {
        if (sourceFolder.empty() || destinationFolder.empty())
            return false;

        if (!fs::exists(sourceFolder) || !fs::is_directory(sourceFolder))
            return false;

        if (IsSamePath(sourceFolder, destinationFolder))
            return false;

        if (IsPathInside(destinationFolder, sourceFolder))
            return false;

        const fs::path destination =
            destinationFolder / sourceFolder.filename();

        if (IsSamePath(sourceFolder, destination))
            return false;

        return true;
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

    const char* GetPendingCreateIcon(PendingCreateType type)
    {
        switch (type)
        {
        case PendingCreateType::Folder:
            return ICON_FA_FOLDER;
        case PendingCreateType::SpriteAtlas:
            return ICON_FA_IMAGES;
        case PendingCreateType::Tilemap:
            return ICON_FA_TABLE_CELLS;
        default:
            return ICON_FA_FILE;
        }
    }

    std::string GetPendingCreateExtension(PendingCreateType type)
    {
        switch (type)
        {
        case PendingCreateType::SpriteAtlas:
            return ".bsa";
        case PendingCreateType::Tilemap:
            return ".btm";
        case PendingCreateType::Material:
            return ".bmat";
        default:
            return "";
        }
    }

    std::string SanitizeFileName(std::string name)
    {
        for (char& c : name)
        {
            switch (c)
            {
            case '\\':
            case '/':
            case ':':
            case '*':
            case '?':
            case '"':
            case '<':
            case '>':
            case '|':
                c = '_';
                break;
            default:
                break;
            }
        }

        while (!name.empty() && (name.back() == ' ' || name.back() == '.'))
            name.pop_back();

        while (!name.empty() && name.front() == ' ')
            name.erase(name.begin());

        return name;
    }

    std::string MakeUniqueName(const fs::path& folder, const std::string& baseName, const std::string& extension)
    {
        std::string cleanBase = SanitizeFileName(baseName);

        if (cleanBase.empty())
            cleanBase = "NewAsset";

        fs::path candidate = folder / (cleanBase + extension);

        if (!fs::exists(candidate))
            return cleanBase;

        for (int i = 1; i < 10000; ++i)
        {
            std::string numbered = cleanBase + "_" + std::to_string(i);
            candidate = folder / (numbered + extension);

            if (!fs::exists(candidate))
                return numbered;
        }

        return cleanBase + "_copy";
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

    void DrawScaledIcon(
        ImDrawList* drawList,
        const char* icon,
        const ImVec2& thumbMin,
        const ImVec2& thumbMax,
        float iconSize,
        ImU32 color)
    {
        const float iconScale = std::clamp(iconSize / 64.0f, 1.8f, 12.5f);
        ImFont* font = ImGui::GetFont();
        const float fontSize = ImGui::GetFontSize() * iconScale;
        const ImVec2 baseIconSize = ImGui::CalcTextSize(icon);

        drawList->AddText(
            font,
            fontSize,
            ImVec2(
                thumbMin.x + (thumbMax.x - thumbMin.x - baseIconSize.x * iconScale) * 0.5f,
                thumbMin.y + (thumbMax.y - thumbMin.y - ImGui::GetFontSize() * iconScale) * 0.42f),
            color,
            icon);
    }

    void SetFolderDragPayload(const fs::path& folderPath)
    {
        FolderDragPayload payload{};

        const std::string path = folderPath.lexically_normal().string();
        strcpy_s(payload.Path, sizeof(payload.Path), path.c_str());

        ImGui::SetDragDropPayload(FolderPayloadName, &payload, sizeof(payload));
    }

    bool FindAssetLogicalPath(AssetHandle handle, std::string& outPath)
    {
        outPath.clear();

        AssetDatabase::Get().ForEachEntry(
            [&outPath, handle](AssetHandle candidate, const std::string& candidatePath)
            {
                if (!outPath.empty())
                    return;

                if (candidate == handle)
                    outPath = candidatePath;
            });

        return !outPath.empty();
    }

    fs::path AssetLogicalToSourcePath(const std::string& logicalPath)
    {
        const auto& roots =
            ServiceLocator::Get<AssetImporterRegistry>().GetAssetRoots();

        if (roots.empty())
            return {};

        return (roots[0].sourceRoot / logicalPath).lexically_normal();
    }

    bool MoveAssetHandleToFolder(EditorCommandQueue& cmd, AssetHandle handle, const fs::path& destinationFolder)
    {
        std::string logicalPath;

        if (!FindAssetLogicalPath(handle, logicalPath))
            return false;

        fs::path source = logicalPath;

        fs::path destination =
            destinationFolder / source.filename();

        if (source == destination)
            return false;

        cmd.Push<ActionCommand>(
            [source, destination]
            {
                AssetDatabase::Get().Move(source, destination);
            });

        return true;
    }

    bool MoveFolderToFolder(EditorCommandQueue& cmd, const fs::path& sourceFolder, const fs::path& destinationFolder)
    {
        if (!CanMoveFolderTo(sourceFolder, destinationFolder))
            return false;

        const fs::path destination =
            destinationFolder / sourceFolder.filename();

        cmd.Push<ActionCommand>([sourceFolder, destination]
            {
                AssetDatabase::Get().MoveRecursively(sourceFolder, destination);
            });
        return true;
    }

    bool AcceptContentDropToFolder(EditorCommandQueue& cmd, const fs::path& destinationFolderFull, const fs::path& destinationFolderLogical)
    {
        bool changed = false;

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(FolderPayloadName))
        {
            const auto* data =
                static_cast<const FolderDragPayload*>(payload->Data);

            if (data && data->Path[0] != '\0')
                changed = MoveFolderToFolder(cmd, data->Path, destinationFolderFull);
        }

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE"))
        {
            if (payload->DataSize == sizeof(AssetHandle))
            {
                const AssetHandle handle =
                    *static_cast<const AssetHandle*>(payload->Data);

                changed = MoveAssetHandleToFolder(cmd, handle, destinationFolderLogical) || changed;
            }
        }

        return changed;
    }
}

ContentBrowser::ContentBrowser(
    EditorContext* pContext,
    const std::string& name,
    AssetContext* pAsset)
    : EditorPanel(pContext, name)
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
    const std::string previousFolder =
        m_CurrentFolder
        ? NormalizePathString(m_CurrentFolder->logicalPath)
        : std::string{};

    std::function<void(FolderNode*)> deleteChildren =
        [&](FolderNode* node)
        {
            if (!node)
                return;

            for (auto& [name, child] : node->children)
            {
                deleteChildren(child);
                delete child;
            }

            node->children.clear();
            node->assets.clear();
        };

    deleteChildren(&m_RootFolder);

    const RuntimeConfig& config = Application::Get().GetDescriptor();

    m_root = config.AssetsRoot;
    m_RootFolder.name = "Content";
    m_RootFolder.fullPath = m_root.string();
    m_RootFolder.logicalPath = "";

    if (!fs::exists(m_root))
    {
        fs::create_directories(m_root);
        AssetDatabase::Get().MarkDirty();
    }

    BuildFoldersFromDisk(m_root.string(), &m_RootFolder);

    AssetDatabase::Get().ForEachEntry(
        [this](AssetHandle, const std::string& assetPath)
        {
            AddAssetToTree(assetPath);
        });

    m_CurrentFolder = &m_RootFolder;

    if (!previousFolder.empty())
    {
        std::function<FolderNode* (FolderNode*)> findByPath =
            [&](FolderNode* node) -> FolderNode*
            {
                if (!node)
                    return nullptr;

                if (NormalizePathString(node->logicalPath) == previousFolder)
                    return node;

                for (auto& [name, child] : node->children)
                {
                    if (FolderNode* found = findByPath(child))
                        return found;
                }

                return nullptr;
            };

        if (FolderNode* restored = findByPath(&m_RootFolder))
            m_CurrentFolder = restored;
    }
}

void ContentBrowser::BuildFoldersFromDisk(
    const std::string& path,
    FolderNode* parent)
{
    if (!fs::exists(path))
        return;

    std::vector<fs::path> folders;

    for (const auto& entry : fs::directory_iterator(path))
    {
        if (entry.is_directory())
            folders.push_back(entry.path());
    }

    std::sort(
        folders.begin(),
        folders.end(),
        [](const fs::path& a, const fs::path& b)
        {
            return a.filename().string() < b.filename().string();
        });

    for (const auto& fullPath : folders)
    {
        const std::string name =
            fullPath.filename().string();

        std::string logical;

        if (parent->logicalPath.empty())
            logical = name;
        else
            logical =
            (fs::path(parent->logicalPath) / name)
            .generic_string();

        FolderNode* node = new FolderNode(
            name,
            fullPath.string(),
            logical);

        parent->children[name] = node;

        BuildFoldersFromDisk(
            fullPath.string(),
            node);
    }
}

void ContentBrowser::AddAssetToTree(const std::string& path)
{
    fs::path p = fs::path(path).lexically_normal();

    if (p.empty())
        return;

    FolderNode* node = &m_RootFolder;

    fs::path parentPath = p.parent_path();

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

    const std::string logical =
        p.generic_string();

    if (std::find(node->assets.begin(), node->assets.end(), logical) == node->assets.end())
        node->assets.push_back(logical);

    std::sort(node->assets.begin(), node->assets.end());
}

void ContentBrowser::CollectAssetsRecursive(FolderNode* node, std::vector<std::string>& out)
{
    if (!node)
        return;

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

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        m_CurrentFolder = node;

    if (ImGui::BeginDragDropTarget())
    {
        bool changed = AcceptContentDropToFolder(*GetContext().GetCommandQueue(), node->fullPath, node->logicalPath);

        ImGui::EndDragDropTarget();

        //if (changed)
        //{
        //    BuildFolderTree();
        //    ImGui::PopStyleVar(3);
        //    return;
        //}
    }

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

    static PendingCreateType pendingCreateType = PendingCreateType::None;
    static char pendingCreateName[128]{};
    static bool focusPendingCreate = false;

    bool rebuildAfterFrame = false;

    auto refreshSoon = [&]()
        {
            //rebuildAfterFrame = true;
        };

    const auto cancelPendingCreate = [&]()
        {
            pendingCreateType = PendingCreateType::None;
            std::memset(pendingCreateName, 0, sizeof(pendingCreateName));
            focusPendingCreate = false;
        };

    const auto startPendingCreate = [&](PendingCreateType type, const char* defaultName)
        {
            pendingCreateType = type;
            std::memset(pendingCreateName, 0, sizeof(pendingCreateName));

            const std::string extension = GetPendingCreateExtension(type);
            const std::string uniqueName = MakeUniqueName(folder->fullPath, defaultName, extension);

            strcpy_s(pendingCreateName, sizeof(pendingCreateName), uniqueName.c_str());
            focusPendingCreate = true;
        };

    const auto commitPendingCreate = [&]()
        {
            std::string name = SanitizeFileName(pendingCreateName);

            if (name.empty())
            {
                cancelPendingCreate();
                return;
            }

            const fs::path folderPath = folder->fullPath;

            switch (pendingCreateType)
            {
            case PendingCreateType::Folder:
            {
                const std::string uniqueName = MakeUniqueName(folderPath, name, "");
                fs::create_directories(folderPath / uniqueName);
                AssetDatabase::Get().MarkDirty();
                break;
            }
            case PendingCreateType::SpriteAtlas:
            {
                const std::string extension = GetPendingCreateExtension(pendingCreateType);
                const std::string uniqueName = MakeUniqueName(folderPath, name, extension);

                ServiceLocator::Get<AssetImporterRegistry>().Export<SpriteAtlasAsset>(folderPath / (uniqueName + extension),0u);
                break;
            }
            case PendingCreateType::Tilemap:
            {
                const std::string extension = GetPendingCreateExtension(pendingCreateType);
                const std::string uniqueName = MakeUniqueName(folderPath, name, extension);

                ServiceLocator::Get<AssetImporterRegistry>().Export<TilemapAsset>(folderPath / (uniqueName + extension), 0u);
                break;
            }
            case PendingCreateType::Material:
            {
                const std::string extension = GetPendingCreateExtension(pendingCreateType);
                const std::string uniqueName = MakeUniqueName(folderPath, name, extension);

                ServiceLocator::Get<AssetImporterRegistry>().Export<MaterialAsset>(folderPath / (uniqueName + extension), 0u);
                break;
            }
            default:
                break;
            }

            cancelPendingCreate();
            refreshSoon();
        };

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

    const bool showFolders = m_SearchBuffer[0] == '\0';
    const bool hasPendingCreate = pendingCreateType != PendingCreateType::None;

    const bool hasContent =
        hasPendingCreate ||
        (showFolders && !folder->children.empty()) ||
        !displayAssets.empty();

    if (!hasContent)
    {
        ImGui::Dummy(ImVec2(0.0f, 48.0f));

        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));

        const char* emptyText = ICON_FA_BOX_OPEN "  Empty folder";
        const ImVec2 textSize = ImGui::CalcTextSize(emptyText);

        ImGui::SetCursorPosX(
            std::max(0.0f, (ImGui::GetContentRegionAvail().x - textSize.x) * 0.5f));

        ImGui::TextUnformatted(emptyText);

        ImGui::PopStyleColor();
    }

    ImGuiTableFlags tableFlags =
        ImGuiTableFlags_NoBordersInBody |
        ImGuiTableFlags_NoPadOuterX |
        ImGuiTableFlags_SizingFixedFit |
        ImGuiTableFlags_NoHostExtendX;

    if (hasContent && ImGui::BeginTable("##ContentGrid", columns, tableFlags))
    {
        for (int col = 0; col < columns; ++col)
            ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed, colWidth);

        if (hasPendingCreate)
        {
            ImGui::PushID("##PendingCreateCard");
            ImGui::TableNextColumn();

            const float cellStartX = ImGui::GetCursorPosX();
            const float cellWidth = ImGui::GetColumnWidth();

            const float cardWidth = iconSize;
            const float cardHeight = iconSize + ImGui::GetTextLineHeight() * 2.8f;
            const float cardX = cellStartX + (cellWidth - cardWidth) * 0.5f;

            ImGui::SetCursorPosX(cardX);

            ImVec2 cardPos = ImGui::GetCursorScreenPos();
            ImVec2 cardMax(cardPos.x + cardWidth, cardPos.y + cardHeight);

            ImGui::InvisibleButton("##PendingCreateHitbox", ImVec2(cardWidth, cardHeight));

            ImDrawList* drawList = ImGui::GetWindowDrawList();

            drawList->AddRect(
                cardPos,
                cardMax,
                Accent(0.95f),
                10.0f,
                0,
                2.0f);

            const float thumbPadding = 10.0f;
            ImVec2 thumbMin(cardPos.x + thumbPadding, cardPos.y + thumbPadding);
            ImVec2 thumbMax(cardPos.x + cardWidth - thumbPadding, cardPos.y + cardWidth - thumbPadding);

            DrawScaledIcon(
                drawList,
                GetPendingCreateIcon(pendingCreateType),
                thumbMin,
                thumbMax,
                iconSize,
                Accent(1.0f));

            ImGui::SetCursorScreenPos(ImVec2(cardPos.x + 6.0f, thumbMax.y + 7.0f));
            ImGui::SetNextItemWidth(cardWidth - 12.0f);

            if (focusPendingCreate)
            {
                ImGui::SetKeyboardFocusHere();
                focusPendingCreate = false;
            }

            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

            const bool commit = ImGui::InputText(
                "##PendingCreateName",
                pendingCreateName,
                sizeof(pendingCreateName),
                ImGuiInputTextFlags_EnterReturnsTrue |
                ImGuiInputTextFlags_AutoSelectAll);

            const bool active = ImGui::IsItemActive();

            ImGui::PopStyleVar();
            ImGui::PopStyleColor();

            if (commit)
                commitPendingCreate();

            if (active && ImGui::IsKeyPressed(ImGuiKey_Escape))
                cancelPendingCreate();

            ImGui::PopID();
        }

        if (showFolders)
        {
            for (auto& [folderName, child] : folder->children)
            {
                ImGui::PushID(folderName.c_str());
                ImGui::TableNextColumn();

                const float cellStartX = ImGui::GetCursorPosX();
                const float cellWidth = ImGui::GetColumnWidth();

                const float cardWidth = iconSize;
                const float cardHeight = iconSize + ImGui::GetTextLineHeight() * 2.4f;

                const float cardX = cellStartX + (cellWidth - cardWidth) * 0.5f;
                ImGui::SetCursorPosX(cardX);

                ImVec2 cardPos = ImGui::GetCursorScreenPos();

                ImGui::InvisibleButton(
                    "##FolderCard",
                    ImVec2(cardWidth, cardHeight));

                const bool hovered = ImGui::IsItemHovered();

                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                {
                    SetFolderDragPayload(child->fullPath);
                    ImGui::Text("%s  %s", ICON_FA_FOLDER, folderName.c_str());
                    ImGui::EndDragDropSource();
                }

                if (ImGui::BeginDragDropTarget())
                {
                    if (AcceptContentDropToFolder(*GetContext().GetCommandQueue(), child->fullPath, child->logicalPath))
                        refreshSoon();

                    ImGui::EndDragDropTarget();
                }

                if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    m_CurrentFolder = child;

                if (hovered)
                    ImGui::SetTooltip("%s", folderName.c_str());

                bool deleteFolder = false;

                if (ImGui::BeginPopupContextItem("##FolderContextMenu"))
                {
                    if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN "  Open"))
                        m_CurrentFolder = child;

                    ImGui::Separator();

                    if (ImGui::MenuItem(ICON_FA_COPY "  Copy"))
                    {
                        m_ClipboardMode = ClipboardMode::Copy;
                        m_ClipboardIsFolder = true;
                        m_ClipboardPath = child->fullPath;
                    }

                    if (ImGui::MenuItem(ICON_FA_SCISSORS "  Cut"))
                    {
                        m_ClipboardMode = ClipboardMode::Cut;
                        m_ClipboardIsFolder = true;
                        m_ClipboardPath = child->fullPath;
                    }

                    ImGui::Separator();

                    if (ImGui::MenuItem(ICON_FA_TRASH_CAN "  Delete Folder"))
                        deleteFolder = true;

                    ImGui::EndPopup();
                }

                ImDrawList* drawList = ImGui::GetWindowDrawList();

                ImVec2 cardMax(
                    cardPos.x + cardWidth,
                    cardPos.y + cardHeight);

                if (hovered)
                {
                    drawList->AddRect(
                        cardPos,
                        cardMax,
                        Accent(0.55f),
                        10.0f,
                        0,
                        1.0f);
                }

                const float thumbPadding = 10.0f;

                ImVec2 thumbMin(
                    cardPos.x + thumbPadding,
                    cardPos.y + thumbPadding);

                ImVec2 thumbMax(
                    cardPos.x + cardWidth - thumbPadding,
                    cardPos.y + cardWidth - thumbPadding);

                DrawScaledIcon(
                    drawList,
                    ICON_FA_FOLDER,
                    thumbMin,
                    thumbMax,
                    iconSize,
                    Accent(1.0f));

                const float labelWidth = cardWidth - 12.0f;
                const float labelX = cardPos.x + 6.0f;
                const float labelY = thumbMax.y + 7.0f;

                std::string displayName =
                    EllipsizeText(folderName, labelWidth);

                drawList->AddText(
                    ImVec2(labelX, labelY),
                    Col(ImGuiCol_Text),
                    displayName.c_str());

                if (deleteFolder)
                {
                    const auto toRem = child->fullPath;
                    GetContext().GetCommandQueue()->Push<ActionCommand>([toRem]
                        {
                            AssetDatabase::Get().RemoveRecursively(toRem);
                        });

                    if (m_CurrentFolder == child)
                        m_CurrentFolder = folder;

                    refreshSoon();
                }

                ImGui::PopID();

                if (rebuildAfterFrame)
                    break;
            }
        }

        if (!rebuildAfterFrame)
        {
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

                bool deleteAsset = false;

                if (ImGui::BeginPopupContextItem("##AssetContextMenu"))
                {
                    if (ImGui::MenuItem(ICON_FA_COPY "  Copy"))
                    {
                        m_ClipboardMode = ClipboardMode::Copy;
                        m_ClipboardIsFolder = false;
                        m_ClipboardPath = assetPath;
                    }

                    if (ImGui::MenuItem(ICON_FA_SCISSORS "  Cut"))
                    {
                        m_ClipboardMode = ClipboardMode::Cut;
                        m_ClipboardIsFolder = false;
                        m_ClipboardPath = assetPath;
                    }

                    ImGui::Separator();

                    if (ImGui::MenuItem(ICON_FA_TRASH_CAN "  Delete Asset"))
                        deleteAsset = true;

                    ImGui::EndPopup();
                }

                if (ImGui::BeginDragDropSource())
                {
                    ImGui::SetDragDropPayload("ASSET_HANDLE", &h, sizeof(AssetHandle));
                    ImGui::Text("%s  %s", GetAssetIcon(assetPath), filename.c_str());
                    ImGui::EndDragDropSource();
                }

                if (hovered)
                    ImGui::SetTooltip("%s", filename.c_str());

                ImDrawList* drawList = ImGui::GetWindowDrawList();

                ImVec2 cardMax(
                    cardPos.x + cardWidth,
                    cardPos.y + cardHeight);

                ImU32 outline =
                    selected ? Accent(0.95f) :
                    hovered ? Accent(0.55f) :
                    Border();

                if (selected || hovered)
                    drawList->AddRect(cardPos, cardMax, outline, 10.0f, 0, selected ? 2.0f : 1.0f);

                const float thumbPadding = 10.0f;

                ImVec2 thumbMin(
                    cardPos.x + thumbPadding,
                    cardPos.y + thumbPadding);

                ImVec2 thumbMax(
                    cardPos.x + cardWidth - thumbPadding,
                    cardPos.y + cardWidth - thumbPadding);

                if (thumbnail.IsValid())
                {
                    drawList->AddImage(
                        thumbnail->GetInstance()->GetRendererID(),
                        thumbMin,
                        thumbMax);
                }
                else
                {
                    DrawScaledIcon(
                        drawList,
                        GetAssetIcon(assetPath),
                        thumbMin,
                        thumbMax,
                        iconSize,
                        selected ? Accent(1.0f) : TextMuted());
                }

                const float labelWidth = cardWidth - 12.0f;
                const float labelX = cardPos.x + 6.0f;
                const float labelY = thumbMax.y + 7.0f;

                std::string displayName =
                    EllipsizeText(filename, labelWidth);

                drawList->AddText(
                    ImVec2(labelX, labelY),
                    selected ? Col(ImGuiCol_Text) : TextMuted(),
                    displayName.c_str());

                if (deleteAsset)
                {
                    if (m_pSelectedAsset && m_pSelectedAsset->Get() == h)
                        m_pSelectedAsset->Set(0u);

                    GetContext().GetCommandQueue()->Push<ActionCommand>([assetPath]
                        {
                            AssetDatabase::Get().Remove(assetPath);
                        });
                    refreshSoon();
                }

                ImGui::PopID();

                if (rebuildAfterFrame)
                    break;
            }
        }

        ImGui::EndTable();
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (AcceptContentDropToFolder(*GetContext().GetCommandQueue(), folder->fullPath, folder->logicalPath))
            refreshSoon();

        ImGui::EndDragDropTarget();
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
            startPendingCreate(PendingCreateType::Folder, "new_folder");

        ImGui::Separator();

        if (ImGui::MenuItem(ICON_FA_IMAGES "  New Sprite Atlas"))
            startPendingCreate(PendingCreateType::SpriteAtlas, "new_sprite");

        if (ImGui::MenuItem(ICON_FA_TABLE_CELLS "  New Tilemap"))
            startPendingCreate(PendingCreateType::Tilemap, "new_tilemap");

        if (ImGui::MenuItem(ICON_FA_TABLE_CELLS "  New Material"))
            startPendingCreate(PendingCreateType::Material, "new_material");

        ImGui::Separator();

        if (ImGui::MenuItem(
            ICON_FA_PASTE "  Paste",
            nullptr,
            false,
            m_ClipboardMode != ClipboardMode::None))
        {
            if (m_ClipboardIsFolder)
            {
                fs::path source = m_ClipboardPath;
                fs::path destination = folder->fullPath / source.filename();

                if (m_ClipboardMode == ClipboardMode::Copy)
                {
                    GetContext().GetCommandQueue()->Push<ActionCommand>([source, destination]
                        {
                            AssetDatabase::Get().CopyRecursively(source, destination);
                        });
                }
                else if (m_ClipboardMode == ClipboardMode::Cut)
                {
                    GetContext().GetCommandQueue()->Push<ActionCommand>([source, destination]
                        {
                            AssetDatabase::Get().MoveRecursively(source, destination);
                        });
                }
            }
            else
            {
                const fs::path source = m_ClipboardPath;
                const fs::path destination = folder->logicalPath / source.filename();

                if (m_ClipboardMode == ClipboardMode::Copy)
                {
                    GetContext().GetCommandQueue()->Push<ActionCommand>([source, destination]
                        {
                            AssetDatabase::Get().Copy(source, destination);
                        });
                }
                else if (m_ClipboardMode == ClipboardMode::Cut)
                {
                    GetContext().GetCommandQueue()->Push<ActionCommand>([source, destination]
                        {
                            AssetDatabase::Get().Move(source, destination);
                        });
                }
            }

            if (m_ClipboardMode == ClipboardMode::Cut)
            {
                m_ClipboardMode = ClipboardMode::None;
                m_ClipboardPath.clear();
            }

            refreshSoon();
        }

        ImGui::EndPopup();
    }

    ImGui::EndChild();
}
