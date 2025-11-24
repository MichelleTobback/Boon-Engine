#pragma once

#include <glm/glm.hpp>

namespace Boon
{
	namespace UBData
	{
		struct Camera
		{
			glm::mat4 ViewProjection;
		};

		struct Object
		{
			glm::mat4 World;
			int ID;
		};
	}
}