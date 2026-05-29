#include "Core/EditorState.h"
#include "Project/ProjectLoader.h"
#include <Core/Application.h>

#include <iostream>

using namespace BoonEditor;

Boon::ProjectConfig CreateDefaultEditorConfig()
{
    Boon::ProjectConfig config{};
    Boon::ProjectLoader::ApplyDefaults(config);

    config.Name = "Boon Editor";
    config.Runtime.GameModule = "Editor";
    config.Runtime.EnabledModules.clear();

    config.Runtime.ProjectRoot = std::filesystem::current_path();
    config.Runtime.AssetsRoot = config.Runtime.ProjectRoot / "Assets";
    config.Runtime.SavedRoot = config.Runtime.ProjectRoot / "Saved";

    config.Runtime.Window.Title = "Boon Editor";

    return config;
}

Boon::ProjectConfig ProjectConfigFromArgs(int argc, char** argv)
{
    if (argc <= 1)
        return CreateDefaultEditorConfig();

    const std::filesystem::path path = argv[1];

    auto result = Boon::ProjectLoader::LoadFromFile(path);

    if (!result.Succeeded())
    {
        std::cerr << "Failed to load project: " << path.string() << "\n";

        return CreateDefaultEditorConfig();
    }

    return result.Value;
}

int Run(int argc, char** argv)
{
    Boon::ProjectConfig config = ProjectConfigFromArgs(argc, argv);
    config.Runtime.Window.bBorderless = true;

    Boon::Application app{ config.Runtime };
    app.Run(std::make_shared<BoonEditor::EditorState>(config));
    return 0;
}

#if defined(BOON_EDITOR_NO_CONSOLE) && defined(_WIN32)

#include <windows.h>

#include <DbgHelp.h>
#include <filesystem>
#include <fstream>

#pragma comment(lib, "Dbghelp.lib")

static LONG WINAPI BoonUnhandledExceptionHandler(EXCEPTION_POINTERS* exceptionInfo)
{
    std::filesystem::create_directories("Saved/Crashes");

    const auto dumpPath = std::filesystem::path("Saved/Crashes/BoonCrash.dmp");
    const auto logPath = std::filesystem::path("Saved/Crashes/BoonCrash.txt");

    {
        std::ofstream log(logPath, std::ios::out | std::ios::trunc);
        log << "Boon crashed\n";
        log << "Exception code: 0x"
            << std::hex
            << exceptionInfo->ExceptionRecord->ExceptionCode
            << "\n";
        log << "Exception address: "
            << exceptionInfo->ExceptionRecord->ExceptionAddress
            << "\n";
    }

    HANDLE file = CreateFileW(
        dumpPath.wstring().c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (file != INVALID_HANDLE_VALUE)
    {
        MINIDUMP_EXCEPTION_INFORMATION dumpInfo{};
        dumpInfo.ThreadId = GetCurrentThreadId();
        dumpInfo.ExceptionPointers = exceptionInfo;
        dumpInfo.ClientPointers = FALSE;

        MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            file,
            MiniDumpWithDataSegs,
            &dumpInfo,
            nullptr,
            nullptr);

        CloseHandle(file);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    SetUnhandledExceptionFilter(BoonUnhandledExceptionHandler);
    return Run(__argc, __argv);
}

#else

int main(int argc, char** argv)
{
    return Run(argc, argv);
}

#endif