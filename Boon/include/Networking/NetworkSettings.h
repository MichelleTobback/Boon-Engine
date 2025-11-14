#pragma once
#include "NetAuthority.h"
#include <string>

namespace Boon
{
	struct NetworkSettings
	{
		ENetDriverMode NetMode = ENetDriverMode::Standalone;

		std::string Ip = "127.0.0.1"; // default local host
		uint16_t Port = 27020u;
	};
}