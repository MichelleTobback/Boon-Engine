#pragma once
#include <box2d/id.h>

namespace Boon
{
	struct Rigidbody2D final
	{
		enum class BodyType 
		{ 
			Static = 0, 
			Dynamic = 1, 
			Kinematic = 2 
		};

		BodyType Type = BodyType::Static;
		bool FixedRotation = false;

		b2BodyId RuntimeBody;
	};
}