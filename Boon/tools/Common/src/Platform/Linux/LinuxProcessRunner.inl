#pragma once

#ifdef __linux__

#include <cerrno>
#include <cstring>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace BoonEditor
{
    ProcessResult ProcessRunner::Run(
        const std::string& command,
        OutputCallback callback,
        const std::filesystem::path& workingDirectory)
    {
        ProcessResult result{};

        int pipeFd[2];

        if (pipe(pipeFd) != 0)
        {
            result.Output = std::string("pipe() failed: ") + std::strerror(errno);
            return result;
        }

        const pid_t pid = fork();

        if (pid < 0)
        {
            result.Output = std::string("fork() failed: ") + std::strerror(errno);

            close(pipeFd[0]);
            close(pipeFd[1]);

            return result;
        }

        if (pid == 0)
        {
            if (!workingDirectory.empty())
            {
                if (chdir(workingDirectory.string().c_str()) != 0)
                    _exit(126);
            }

            dup2(pipeFd[1], STDOUT_FILENO);
            dup2(pipeFd[1], STDERR_FILENO);

            close(pipeFd[0]);
            close(pipeFd[1]);

            execl(
                "/bin/sh",
                "sh",
                "-c",
                command.c_str(),
                static_cast<char*>(nullptr));

            _exit(127);
        }

        close(pipeFd[1]);

        char buffer[4096];

        while (true)
        {
            const ssize_t bytesRead =
                read(pipeFd[0], buffer, sizeof(buffer) - 1);

            if (bytesRead > 0)
            {
                buffer[bytesRead] = '\0';

                std::string chunk(buffer);
                result.Output += chunk;

                if (callback)
                    callback(chunk);

                continue;
            }

            if (bytesRead == 0)
                break;

            if (errno == EINTR)
                continue;

            result.Output += std::string("read() failed: ") + std::strerror(errno);
            break;
        }

        close(pipeFd[0]);

        int status = 0;

        while (waitpid(pid, &status, 0) < 0)
        {
            if (errno == EINTR)
                continue;

            result.Output += std::string("waitpid() failed: ") + std::strerror(errno);
            return result;
        }

        if (WIFEXITED(status))
        {
            result.ExitCode = WEXITSTATUS(status);
        }
        else if (WIFSIGNALED(status))
        {
            result.ExitCode = 128 + WTERMSIG(status);
        }
        else
        {
            result.ExitCode = -1;
        }

        return result;
    }
}

#endif