#include "pch.h"
#include "Time.h"

namespace Toad
{

static sf::Clock startup_clock;
static sf::Clock delta_clock;
static sf::Time delta_time;
static float time_scale = 1.f;
static float fixed_delta_time = 0.02f;

const sf::Time& Time::GetDeltaTimeRaw()
{
	return delta_time;
}

float Time::GetDeltaTime()
{
	return delta_time.asSeconds() * time_scale;
}

void Time::SetFixedDeltaTime(float fdt)
{
	fixed_delta_time = fdt;
}

float Time::GetFixedDeltaTime()
{
	return fixed_delta_time;
}

float Time::GetTime()
{
	return startup_clock.getElapsedTime().asSeconds();
}

void Time::SetTimeScale(float scale)
{
	time_scale = scale;
}

void Time::UpdateDeltaTime()
{
	delta_time = delta_clock.restart();
}

}