#pragma once
#include <cstdint> //uint64_t

namespace Boon
{
	class Random final
	{
	public:
		/**
		 * @brief Returns a uniformly distributed 64-bit unsigned random value.
		 * @return Random uint64_t.
		 */
		static uint64_t GetRandomUint64();

		/**
		 * @brief Returns a random integer using the default engine.
		 * @return Random int value.
		 */
		static int GetRandomInt();

		/**
		 * @brief Returns a random integer within [min, max].
		 * @param min Inclusive lower bound.
		 * @param max Inclusive upper bound.
		 * @return Random int in range.
		 */
		static int GetRandomIntInRange(int min, int max);

		/**
		 * @brief Returns a random float in range [0.0f, 1.0f).
		 * @return Random float.
		 */
		static float GetRandomFloat();

		/**
		 * @brief Returns a random float within [min, max].
		 * @param min Inclusive lower bound.
		 * @param max Inclusive upper bound.
		 * @return Random float in range.
		 */
		static float GetRandomFloatInRange(float min, float max);
	private:
	};
}