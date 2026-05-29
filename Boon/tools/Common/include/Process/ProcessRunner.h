#pragma once

#include <filesystem>
#include <functional>
#include <string>

namespace BoonEditor
{
    struct ProcessResult
    {
        int ExitCode = -1;
        std::string Output;
    };

    class ProcessRunner
    {
    public:
        using OutputCallback = std::function<void(const std::string&)>;

        static ProcessResult Run(
            const std::string& command,
            OutputCallback callback = nullptr);

        static ProcessResult Run(
            const std::string& command,
            OutputCallback callback,
            const std::filesystem::path& workingDirectory);
    };
}