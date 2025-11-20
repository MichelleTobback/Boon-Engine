#pragma once
#include "EditorPanel.h"
#include "Assets/AssetDatabase.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

namespace BoonEditor
{
    class ContentBrowser : public EditorPanel
    {
    public:
        ContentBrowser(const std::string& name, DragDropRouter* pRouter);

        virtual void Update() override;

    protected:
        virtual void OnRenderUI() override;

    private:
        struct FolderNode
        {
            std::string name;          // Folder name only (Textures, Scripts, etc.)
            std::string fullPath;      // Full OS path
            std::vector<std::string> assets;          // Only assets from AssetDatabase
            std::unordered_map<std::string, FolderNode*> children;  // Real folders

            FolderNode(const std::string& name = "", const std::string& full = "")
                : name(name), fullPath(full) {
            }
        };

        FolderNode m_RootFolder;
        FolderNode* m_CurrentFolder = nullptr;
        float m_HierarchyWidth = 250.f;

        char m_SearchBuffer[256];

        void BuildFolderTree();
        void BuildFoldersFromDisk(const std::string& path, FolderNode* parent);
        void AddAssetToTree(const std::string& path);
        void CollectAssetsRecursive(FolderNode* node, std::vector<std::string>& out);

        void DrawFolderTree(FolderNode* node);
        void DrawContentArea(FolderNode* folder);
    };
}
