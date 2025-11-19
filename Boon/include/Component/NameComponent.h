#pragma once
#include "Core/Boon.h"

#include <string>

namespace Boon
{
	BCLASS(HideInInspector)
	struct NameComponent final
	{
		NameComponent() = default;
		NameComponent(const std::string& name)
			: Name(name) {}

		BPROPERTY()
		std::string Name;
	};
}