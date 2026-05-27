#pragma once

#include <functional>
#include <string>

namespace BoonEditor
{
	struct ProcessResult
	{
		int ExitCode = -1;
		std::string Output;
	};

	class ProcessRunner final
	{
	public:
		using OutputCallback = std::function<void(const std::string&)>;

		static ProcessResult Run(const std::string& command, OutputCallback callback = nullptr);
	};
}