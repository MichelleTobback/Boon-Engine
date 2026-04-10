#pragma once
#include "Core/ServiceLocator.h"

#include <memory>
#include <string>
#include <vector>
#include <format>

namespace Boon
{
	enum class LogLevel
	{
		Info,
		Warning,
		Error
	};

	class ILogSink
	{
	public:
		virtual ~ILogSink() = default;
		virtual void Write(LogLevel level, const std::string& message) = 0;
	};

    class Logger final
    {
    public:
        static void Init();

        void AddSink(const std::shared_ptr<ILogSink>& sink);
        void RemoveSink(ILogSink* sink);

        void Log(const std::string& msg);
        void Warn(const std::string& msg);
        void Error(const std::string& msg);

        template<typename... TArgs>
        void LogFmt(std::format_string<TArgs...> fmt, TArgs&&... args)
        {
            Log(std::format(fmt, std::forward<TArgs>(args)...));
        }

        template<typename... TArgs>
        void WarnFmt(std::format_string<TArgs...> fmt, TArgs&&... args)
        {
            Warn(std::format(fmt, std::forward<TArgs>(args)...));
        }

        template<typename... TArgs>
        void ErrorFmt(std::format_string<TArgs...> fmt, TArgs&&... args)
        {
            Error(std::format(fmt, std::forward<TArgs>(args)...));
        }

    private:
        void Write(LogLevel level, const std::string& msg);

        std::vector<std::shared_ptr<ILogSink>> m_Sinks;
    };

    class Log
    {
    public:
        static void Info(const std::string& msg);
        static void Warn(const std::string& msg);
        static void Error(const std::string& msg);

        template<typename... TArgs>
        static void Info(std::format_string<TArgs...> fmt, TArgs&&... args)
        {
            Info(std::format(fmt, std::forward<TArgs>(args)...));
        }

        template<typename... TArgs>
        static void Warn(std::format_string<TArgs...> fmt, TArgs&&... args)
        {
            Warn(std::format(fmt, std::forward<TArgs>(args)...));
        }

        template<typename... TArgs>
        static void Error(std::format_string<TArgs...> fmt, TArgs&&... args)
        {
            Error(std::format(fmt, std::forward<TArgs>(args)...));
        }

    private:
        static void FallbackWrite(std::string_view prefix, std::string_view msg);
    };
}

#define BOON_INIT_LOGGER() ::Boon::Logger::Init()

#define BOON_LOG(...) ::Boon::Log::Info(__VA_ARGS__)
#define BOON_LOG_WARN(...) ::Boon::Log::Warn(__VA_ARGS__)
#define BOON_LOG_ERROR(...) ::Boon::Log::Error(__VA_ARGS__)