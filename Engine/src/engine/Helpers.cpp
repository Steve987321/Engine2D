#include "pch.h"
#include "Helpers.h"

namespace Toad
{
	int rand_int(int min, int max)
	{
		assert(min <= max);
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> dis(min, max);
		return dis(gen);
	}

	float rand_float(float min, float max)
	{
		assert(min <= max);
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dis(min, max);
		return dis(gen);
	}
}