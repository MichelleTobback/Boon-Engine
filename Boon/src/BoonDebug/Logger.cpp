#include "BoonDebug/Logger.h"

#include <iostream>
#include <memory>

#include <iostream>
#include <mutex>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace Boon;

namespace
{
	class StdoutLogSink final : public ILogSink
	{
	public:
		virtual void Write(LogLevel level, const std::string& message) override
		{
			switch (level)
			{
			case LogLevel::Info:
				std::cout << "[Info] " << message << '\n';
				break;
			case LogLevel::Warning:
				std::cout << "[Warning] " << message << '\n';
				break;
			case LogLevel::Error:
				std::cout << "[Error] " << message << '\n';
				break;
			}
		}
	};
}

void Logger::Init()
{
	auto logger = std::make_shared<Logger>();
	logger->AddSink(std::make_shared<StdoutLogSink>());
	ServiceLocator::Register(logger);
}

void Logger::AddSink(const std::shared_ptr<ILogSink>& sink)
{
	m_Sinks.push_back(sink);
}

void Boon::Logger::RemoveSink(ILogSink* sink)
{
	m_Sinks.erase(
		std::remove_if(m_Sinks.begin(), m_Sinks.end(),
			[sink](const std::shared_ptr<ILogSink>& s)
			{
				return s.get() == sink;
			}),
		m_Sinks.end());
}

void Logger::Log(const std::string& msg)
{
	Write(LogLevel::Info, msg);
}

void Logger::Warn(const std::string& msg)
{
	Write(LogLevel::Warning, msg);
}

void Logger::Error(const std::string& msg)
{
	Write(LogLevel::Error, msg);
}

void Logger::Write(LogLevel level, const std::string& msg)
{
	for (const auto& sink : m_Sinks)
	{
		sink->Write(level, msg);
	}
}

void Log::Info(const std::string& msg)
{
    if (ServiceLocator::Has<Logger>())
    {
        ServiceLocator::Get<Logger>().Log(msg);
        return;
    }

    FallbackWrite("[Log] ", msg);
}

void Log::Warn(const std::string& msg)
{
    if (ServiceLocator::Has<Logger>())
    {
        ServiceLocator::Get<Logger>().Warn(msg);
        return;
    }

    FallbackWrite("[Warn] ", msg);
}

void Log::Error(const std::string& msg)
{
    if (ServiceLocator::Has<Logger>())
    {
        ServiceLocator::Get<Logger>().Error(msg);
        return;
    }

    FallbackWrite("[Error] ", msg);
}

void Log::FallbackWrite(std::string_view prefix, std::string_view msg)
{
    static std::mutex s_Mutex;
    std::scoped_lock lock(s_Mutex);

    const std::string line = std::string(prefix) + std::string(msg) + "\n";

#ifdef _WIN32
    OutputDebugStringA(line.c_str());
#endif

    std::cerr << line;
}