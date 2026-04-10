#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <span>

namespace Boon
{
    class FileSystem
    {
    public:
        using Path = std::filesystem::path;

        struct FileFilter
        {
            std::wstring name;
            std::wstring pattern; // Example: L"*.cpp;*.h"
        };

        struct OpenDialogOptions
        {
            std::wstring title;
            std::wstring acceptLabel;
            Path initialPath;
            std::vector<FileFilter> filters;
            bool allowMultiSelect = false;
        };

        static Path OpenFolderDialog();
        static Path OpenFolderDialog(const OpenDialogOptions& options);

        static Path OpenFileDialog();
        static Path OpenFileDialog(const OpenDialogOptions& options);

        static std::vector<Path> OpenFilesDialog();
        static std::vector<Path> OpenFilesDialog(const OpenDialogOptions& options);
    };
}