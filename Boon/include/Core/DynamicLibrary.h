#pragma once

#include <filesystem>
#include <string>

namespace Boon
{
    class DynamicLibrary
    {
    public:
        DynamicLibrary() = default;
        ~DynamicLibrary();

        // Non-copyable
        DynamicLibrary(const DynamicLibrary&) = delete;
        DynamicLibrary& operator=(const DynamicLibrary&) = delete;

        // Movable
        DynamicLibrary(DynamicLibrary&& other) noexcept;
        DynamicLibrary& operator=(DynamicLibrary&& other) noexcept;

        // Load / unload
        bool Load(const std::filesystem::path& path);
        void Unload();

        // Symbol lookup
        void* GetSymbol(const char* name) const;

        // Info
        bool IsLoaded() const { return m_Handle != nullptr; }
        const std::filesystem::path& GetPath() const { return m_Path; }

    private:
        void* m_Handle = nullptr;
        std::filesystem::path m_Path;
    };
}