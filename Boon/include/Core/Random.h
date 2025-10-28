#pragma once
#include <cstdint> //uint64_t

namespace Boon
{
	class Random final
	{
	public:
		static uint64_t GetRandomUint64();
		static int GetRandomInt();
		static int GetRandomIntInRange(int min, int max);
		static float GetRandomFloat();
		static float GetRandomFloatInRange(float min, float max);
	private:
	};
}