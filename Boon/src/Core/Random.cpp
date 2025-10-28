#include "Core/Random.h"

#include <random>

namespace Boon
{
	static std::random_device s_RandomDevice{};
	static std::mt19937_64& GetRandomEngine64()
	{
		static std::mt19937_64 sRandomEngine(s_RandomDevice());
		return sRandomEngine;
	}
	static std::mt19937& GetRandomEngine()
	{
		static std::mt19937 sRandomEngine(s_RandomDevice());
		return sRandomEngine;
	}
}

uint64_t Boon::Random::GetRandomUint64()
{
	static std::uniform_int_distribution<uint64_t> sUniformDist{};

	return sUniformDist(GetRandomEngine64());
}

int Boon::Random::GetRandomInt()
{
	return GetRandomIntInRange(INT32_MIN, INT32_MAX);
}

int Boon::Random::GetRandomIntInRange(int min, int max)
{
	static std::uniform_int_distribution<int> sUniformDist(min, max);

	return sUniformDist(GetRandomEngine());
}

float Boon::Random::GetRandomFloat()
{
	return GetRandomFloatInRange(FLT_MIN, FLT_MAX);
}

float Boon::Random::GetRandomFloatInRange(float min, float max)
{
	static std::uniform_real_distribution<float> sUniformDist(min, max);

	return sUniformDist(GetRandomEngine());
}



