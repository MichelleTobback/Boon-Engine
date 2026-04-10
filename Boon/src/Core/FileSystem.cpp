#include "Core/FileSystem.h"

#if defined(_WIN32)

#include <windows.h>
#include <shobjidl.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace Boon
{
    bool CreateShellItemFromPath(const FileSystem::Path& path, IShellItem** item)
    {
        if (path.empty())
            return false;

        return SUCCEEDED(
            SHCreateItemFromParsingName(path.c_str(), nullptr, IID_PPV_ARGS(item)));
    }

    FileSystem::Path GetPathFromShellItem(IShellItem* item)
    {
        if (!item)
            return {};

        PWSTR rawPath = nullptr;
        const HRESULT hr = item->GetDisplayName(SIGDN_FILESYSPATH, &rawPath);
        if (FAILED(hr) || !rawPath)
            return {};

        std::filesystem::path result(rawPath);
        CoTaskMemFree(rawPath);
        return result;
    }

    std::vector<FileSystem::Path> ShowOpenDialog(const FileSystem::OpenDialogOptions& options, DWORD extraFlags)
    {
        std::vector<FileSystem::Path> paths;

        ComPtr<IFileOpenDialog> dialog;
        HRESULT hr = CoCreateInstance(
            CLSID_FileOpenDialog,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&dialog));

        if (FAILED(hr))
            return paths;

        DWORD flags = 0;
        hr = dialog->GetOptions(&flags);
        if (FAILED(hr))
            return paths;

        if (options.allowMultiSelect)
            extraFlags |= FOS_ALLOWMULTISELECT;

        hr = dialog->SetOptions(flags | extraFlags);
        if (FAILED(hr))
            return paths;

        if (!options.title.empty())
            dialog->SetTitle(options.title.c_str());

        if (!options.acceptLabel.empty())
            dialog->SetOkButtonLabel(options.acceptLabel.c_str());

        if (!options.filters.empty())
        {
            std::vector<COMDLG_FILTERSPEC> nativeFilters;
            nativeFilters.reserve(options.filters.size());

            for (const auto& filter : options.filters)
            {
                nativeFilters.push_back(COMDLG_FILTERSPEC{filter.name.c_str(),filter.pattern.c_str()});
            }

            hr = dialog->SetFileTypes(static_cast<UINT>(nativeFilters.size()), nativeFilters.data());

            if (FAILED(hr))
                return paths;

            dialog->SetFileTypeIndex(1);
        }

        if (!options.initialPath.empty())
        {
            ComPtr<IShellItem> folder;
            if (CreateShellItemFromPath(options.initialPath, &folder))
            {
                dialog->SetDefaultFolder(folder.Get());
            }
        }

        hr = dialog->Show(nullptr);
        if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
            return {};

        if (FAILED(hr))
            return {};

        if (options.allowMultiSelect)
        {
            ComPtr<IShellItemArray> items;
            hr = dialog->GetResults(&items);
            if (FAILED(hr))
                return {};

            DWORD count = 0;
            hr = items->GetCount(&count);
            if (FAILED(hr))
                return {};

            paths.reserve(count);

            for (DWORD i = 0; i < count; ++i)
            {
                ComPtr<IShellItem> item;
                if (SUCCEEDED(items->GetItemAt(i, &item)))
                {
                    auto path = GetPathFromShellItem(item.Get());
                    if (!path.empty())
                        paths.push_back(std::move(path));
                }
            }
        }
        else
        {
            ComPtr<IShellItem> item;
            hr = dialog->GetResult(&item);
            if (FAILED(hr))
                return {};

            auto path = GetPathFromShellItem(item.Get());
            if (!path.empty())
                paths.push_back(std::move(path));
        }

        return paths;
    }

    FileSystem::OpenDialogOptions DefaultFolderOptions()
    {
        FileSystem::OpenDialogOptions options;
        options.title = L"Select Folder";
        options.acceptLabel = L"Select Folder";
        return options;
    }

    FileSystem::OpenDialogOptions DefaultFileOptions()
    {
        FileSystem::OpenDialogOptions options;
        options.title = L"Select File";
        options.filters = {
            { L"All Files", L"*.*" }
        };
        return options;
    }

    FileSystem::Path FileSystem::OpenFolderDialog()
    {
        return OpenFolderDialog(DefaultFolderOptions());
    }

    FileSystem::Path FileSystem::OpenFolderDialog(const OpenDialogOptions& options)
    {
        auto results = ShowOpenDialog(
            options,
            FOS_PICKFOLDERS | FOS_PATHMUSTEXIST);

        return results.empty() ? Path{} : results.front();
    }

    FileSystem::Path FileSystem::OpenFileDialog()
    {
        return OpenFileDialog(DefaultFileOptions());
    }

    FileSystem::Path FileSystem::OpenFileDialog(const OpenDialogOptions& options)
    {
        OpenDialogOptions effective = options;
        effective.allowMultiSelect = false;

        auto results = ShowOpenDialog(effective,
            FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST);

        return results.empty() ? Path{} : results.front();
    }

    std::vector<FileSystem::Path> FileSystem::OpenFilesDialog()
    {
        return OpenFilesDialog(DefaultFileOptions());
    }

    std::vector<FileSystem::Path> FileSystem::OpenFilesDialog(const OpenDialogOptions& options)
    {
        OpenDialogOptions effective = options;
        effective.allowMultiSelect = true;

        return ShowOpenDialog(effective,
            FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST);
    }
}

#else

namespace Boon
{
    Path FileSystem::OpenFolderDialog(){}
    Path FileSystem::OpenFolderDialog(const OpenDialogOptions& options){}
    Path FileSystem::OpenFileDialog(){}
    Path FileSystem::OpenFileDialog(const OpenDialogOptions& options){}
    std::vector<Path> FileSystem::OpenFilesDialog(){}
    std::vector<Path> FileSystem::OpenFilesDialog(const OpenDialogOptions& options){}
}
#endif