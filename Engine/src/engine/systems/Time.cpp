#include "pch.h"
#include "Time.h"

namespace Toad
{

float Time::GetDeltaTime()
{
	return m_deltaTime.asSeconds() * scale;
}

void Time::UpdateDeltaTime()
{
	m_deltaTime = m_deltaClock.restart();
}

}