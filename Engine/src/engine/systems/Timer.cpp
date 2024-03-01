#include "pch.h"
#include "Timer.h"

namespace Toad
{

Timer::Timer(bool start)
{
	if (start)
	{
		Start();
	}
}

Timer::~Timer() = default;

void Timer::Start()
{
	m_start = TClock::now();
}

}
