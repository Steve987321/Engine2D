#include "pch.h"
#include "Helpers.h"

namespace Toad
{
	std::random_device rd;
	std::mt19937 gen(rd());
	
	int rand_int(int min, int max)
	{
		assert(min <= max);
		std::uniform_int_distribution<int> dis(min, max);
		return dis(gen);
	}

	float rand_float(float min, float max)
	{
		assert(min <= max);
		std::uniform_real_distribution<float> dis(min, max);
		return dis(gen);
	}

	std::string get_date_str(std::string_view format)
	{
		std::ostringstream ss;
		std::string time;

		auto t = std::time(nullptr);
		tm newtime{};

#ifdef _WIN32
		localtime_s(&newtime, &t);
#else
		localtime_r(&t, &newtime);
#endif

		ss << std::put_time(&newtime, format.data());
		return ss.str();
	}

	ENGINE_API float distance(const Vec2f& a, const Vec2f& b)
	{
		return std::sqrt(pow(b.x - a.x, 2) + pow(b.y - a.y, 2));
	}

}
