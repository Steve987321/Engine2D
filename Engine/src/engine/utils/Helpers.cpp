#include "pch.h"
#include "Helpers.h"

namespace Toad
{
	std::random_device rd;
	std::mt19937 gen(rd());
	
	int RandInt(int min, int max)
	{
		assert(min <= max);
		std::uniform_int_distribution<int> dis(min, max);
		return dis(gen);
	}

	float RandFloat(float min, float max)
	{
		assert(min <= max);
		std::uniform_real_distribution<float> dis(min, max);
		return dis(gen);
	}

	std::string GetDateAsString(std::string_view format)
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

    std::string ToLower(std::string_view str) 
    {
        std::string s {str};
        std::ranges::transform(s, s.begin(), [](unsigned char c){return std::tolower(c);});
        return s;
    }

        std::filesystem::path GetExePath()
	{
#ifdef _WIN32
		char path[MAX_PATH];
		GetModuleFileNameA(NULL, path, MAX_PATH);
		return path;
#else
		char path[MAX_PATH];
		uint32_t size = MAX_PATH;
		_NSGetExecutablePath(path, &size);
		return path;
#endif
	}

    void PathToPreferred(std::string& path_str)
    {
#ifdef __APPLE__
        std::ranges::replace(path_str, '\\', PATH_SEPARATOR);
#else
        std::ranges::replace(path_str, '/', PATH_SEPARATOR);
#endif
    }

	float DegToRad(float degrees)
	{
		return degrees * (std::numbers::pi_v<float> / 180.f);
	}

	float RadToDeg(float radians)
	{
		return radians * (180.f / std::numbers::pi_v<float>);
	}

	std::string GetFileContents(const char* file)
	{
		std::ifstream f(file);
		if (f.is_open())
		{
			std::stringstream ss;
			ss << f.rdbuf();
			f.close();
			return ss.str();
		}

		return "";
	}
}
