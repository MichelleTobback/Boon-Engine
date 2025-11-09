#pragma once
#include "Component/Rigidbody2D.h"
#include "Component/BoxCollider2D.h"
#include <box2d/box2d.h>

namespace Boon
{
	inline b2BodyType Rigidbody2DTypeToBox2DBody(Rigidbody2D::BodyType bodyType)
	{
		switch (bodyType)
		{
		case Rigidbody2D::BodyType::Static:    return b2_staticBody;
		case Rigidbody2D::BodyType::Dynamic:   return b2_dynamicBody;
		case Rigidbody2D::BodyType::Kinematic: return b2_kinematicBody;
		}

		return b2_staticBody;
	}

	inline Rigidbody2D::BodyType Rigidbody2DTypeFromBox2DBody(b2BodyType bodyType)
	{
		switch (bodyType)
		{
		case b2_staticBody:    return Rigidbody2D::BodyType::Static;
		case b2_dynamicBody:   return Rigidbody2D::BodyType::Dynamic;
		case b2_kinematicBody: return Rigidbody2D::BodyType::Kinematic;
		}

		return Rigidbody2D::BodyType::Static;
	}
}