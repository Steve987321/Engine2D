#include "pch.h"
#include "Timer.h"

namespace Toad
{

Timer::Timer(std::chrono::time_point<TClock> starting_point)
{
	m_start = starting_point;
}

Timer::Timer(bool start)
{
	if (start)
	{
		Start();
	}
}

Timer::Timer() = default;

Timer::~Timer() = default;

void Timer::Start()
{
	m_start = TClock::now();
}

}
