#pragma once

#include "EngineCore.h"

namespace Toad
{

class ENGINE_API Time
{
public:
	friend class Engine;

	static inline float scale = 1.f;
	static inline float fixed_delta_time = 0.02f;
	static float GetDeltaTime();

private:
	static void UpdateDeltaTime();

	static inline sf::Clock m_deltaClock;
	static inline sf::Time m_deltaTime;
};

}