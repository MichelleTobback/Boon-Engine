#include "Utils/ProcessRunner.h"

#ifdef _WIN32

#include <windows.h>

namespace BoonEditor
{
	ProcessResult ProcessRunner::Run(
		const std::string& command,
		OutputCallback callback)
	{
		ProcessResult result{};

		SECURITY_ATTRIBUTES sa{};
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.bInheritHandle = TRUE;

		HANDLE readPipe = nullptr;
		HANDLE writePipe = nullptr;

		if (!CreatePipe(&readPipe, &writePipe, &sa, 0))
			return result;

		SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);

		STARTUPINFOA si{};
		si.cb = sizeof(STARTUPINFOA);
		si.dwFlags |= STARTF_USESTDHANDLES;
		si.hStdOutput = writePipe;
		si.hStdError = writePipe;

		PROCESS_INFORMATION pi{};

		std::string cmd = command;

		if (!CreateProcessA(
			nullptr,
			cmd.data(),
			nullptr,
			nullptr,
			TRUE,
			CREATE_NO_WINDOW,
			nullptr,
			nullptr,
			&si,
			&pi))
		{
			CloseHandle(readPipe);
			CloseHandle(writePipe);
			return result;
		}

		CloseHandle(writePipe);

		char buffer[4096];
		DWORD bytesRead = 0;

		while (ReadFile(readPipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0)
		{
			buffer[bytesRead] = '\0';

			std::string chunk(buffer);

			result.Output += chunk;

			if (callback)
				callback(chunk);
		}

		WaitForSingleObject(pi.hProcess, INFINITE);

		DWORD exitCode = 0;
		GetExitCodeProcess(pi.hProcess, &exitCode);

		result.ExitCode = static_cast<int>(exitCode);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CloseHandle(readPipe);

		return result;
	}
}

#endif