#pragma once

#include <EngineCore.h>

namespace Toad
{

class ENGINE_API Timer
{
public:
	using TClock = std::chrono::high_resolution_clock;

	explicit Timer(bool start = false);
	~Timer();

	void Start();

	template<typename TDur = std::chrono::milliseconds>
	float Elapsed() const
	{
		return (float)std::chrono::duration_cast<TDur>(TClock::now() - m_start).count();
	}

private:
	std::chrono::time_point<TClock> m_start;
};

}