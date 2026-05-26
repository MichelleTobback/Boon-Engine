#pragma once

namespace Boon
{
	enum class BlendMode
	{
		None,
		Alpha,
		Additive
	};

	enum class DepthMode
	{
		Disabled,
		Read,
		ReadWrite
	};

	enum class CullMode
	{
		None,
		Back,
		Front
	};

	enum class PrimitiveType
	{
		Triangles,
		Lines
	};
}