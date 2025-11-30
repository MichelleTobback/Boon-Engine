#include "BoonDebug/Logger.h"
#include <iostream>

void Boon::Logger::Init()
{
	ServiceLocator::Register(std::make_shared<Logger>());
}

void Boon::Logger::Log(const std::string& msg)
{
	std::cout << "[Info] " << msg << '\n';
}

void Boon::Logger::Warn(const std::string& msg)
{
	std::cout << "[Warning] " << msg << '\n';
}

void Boon::Logger::Error(const std::string& msg)
{
	std::cout << "[Error] " << msg << '\n';
}
