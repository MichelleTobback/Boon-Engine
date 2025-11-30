#include "Core/ServiceLocator.h"
#include <string>

namespace Boon
{
	class Logger final
	{
	public:
		static void Init();

		void Log(const std::string& msg);
		void Warn(const std::string& msg);
		void Error(const std::string& msg);
	};
}

#define BOON_INIT_LOGGER() ::Boon::Logger::Init()

#define BOON_LOG(...) ::Boon::ServiceLocator::Get<Boon::Logger>().Log(__VA_ARGS__)
#define BOON_LOG_WARN(...) ::Boon::ServiceLocator::Get<Boon::Logger>().Warn(__VA_ARGS__)
#define BOON_LOG_ERROR(...) ::Boon::ServiceLocator::Get<Boon::Logger>().Error(__VA_ARGS__)