#pragma once

#include <EngineCore.h>

namespace Toad
{

class Timer
{
public:
	using TClock = std::chrono::high_resolution_clock;

	ENGINE_API explicit Timer(bool start = false);
	ENGINE_API ~Timer();

	ENGINE_API void Start();

	template<typename TDur = std::chrono::milliseconds>
	ENGINE_API float Elapsed() const
	{
		return (float)std::chrono::duration_cast<TDur>(TClock::now() - m_start).count();
	}

private:
	std::chrono::time_point<TClock> m_start;
};

}