#pragma once

#include <string>

namespace Boon
{
	struct NameComponent final
	{
		NameComponent() = default;
		NameComponent(const std::string& name)
			: Name(name) {}

		std::string Name;
	};
}