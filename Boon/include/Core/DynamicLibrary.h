#pragma once

#include <filesystem>
#include <string>

namespace Boon
{
    class DynamicLibrary
    {
    public:
        DynamicLibrary() = default;

        /**
         * @brief Destroy the DynamicLibrary and unload if still loaded.
         */
        ~DynamicLibrary();

        // Non-copyable
        DynamicLibrary(const DynamicLibrary&) = delete;
        DynamicLibrary& operator=(const DynamicLibrary&) = delete;

        // Movable
        DynamicLibrary(DynamicLibrary&& other) noexcept;
        DynamicLibrary& operator=(DynamicLibrary&& other) noexcept;

        // Load / unload
        /**
         * @brief Load a dynamic library from the given filesystem path.
         *
         * @param path Path to the library file to load.
         * @return true if the library was successfully loaded, false otherwise.
         */
        bool Load(const std::filesystem::path& path);

        /**
         * @brief Unload the library if it is loaded.
         */
        void Unload();

        // Symbol lookup
        /**
         * @brief Lookup a symbol by name in the loaded library.
         *
         * @param name Null-terminated symbol name to find.
         * @return Pointer to the symbol, or nullptr if not found or not loaded.
         */
        void* GetSymbol(const char* name) const;

        // Info
        bool IsLoaded() const { return m_Handle != nullptr; }
        const std::filesystem::path& GetPath() const { return m_Path; }

    private:
        void* m_Handle = nullptr;
        std::filesystem::path m_Path;
    };
}