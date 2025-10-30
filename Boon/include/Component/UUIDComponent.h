#pragma once

#include "Core/UUID.h"

namespace Boon
{
	struct UUIDComponent final
	{
		UUIDComponent() = default;
		UUIDComponent(UUID uuid)
			: Uuid(uuid){}

		UUID Uuid;
	};
}