#include "Core/DynamicLibrary.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace Boon
{
    static std::filesystem::path GetExecutableDirectory()
    {
        wchar_t buffer[MAX_PATH];
        GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        return std::filesystem::path(buffer).parent_path();
    }

    DynamicLibrary::~DynamicLibrary()
    {
        Unload();
    }

    DynamicLibrary::DynamicLibrary(DynamicLibrary&& other) noexcept
    {
        m_Handle = other.m_Handle;
        m_Path = std::move(other.m_Path);

        other.m_Handle = nullptr;
    }

    DynamicLibrary& DynamicLibrary::operator=(DynamicLibrary&& other) noexcept
    {
        if (this != &other)
        {
            Unload();

            m_Handle = other.m_Handle;
            m_Path = std::move(other.m_Path);

            other.m_Handle = nullptr;
        }
        return *this;
    }

    bool DynamicLibrary::Load(const std::filesystem::path& path)
    {
        Unload();

        std::filesystem::path fullPath = GetExecutableDirectory() / path;

#ifdef _WIN32
        m_Handle = static_cast<void*>(LoadLibraryA(fullPath.string().c_str()));
        if (!m_Handle)
            return false;
#else
        // Linux/macOS later: dlopen
        return false;
#endif

        m_Path = fullPath;
        return true;
    }

    void DynamicLibrary::Unload()
    {
        if (!m_Handle)
            return;

#ifdef _WIN32
        FreeLibrary(static_cast<HMODULE>(m_Handle));
#endif

        m_Handle = nullptr;
        m_Path.clear();
    }

    void* DynamicLibrary::GetSymbol(const char* name) const
    {
        if (!m_Handle)
            return nullptr;

#ifdef _WIN32
        return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(m_Handle), name));
#else
        return nullptr;
#endif
    }
}