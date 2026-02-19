#pragma once

#include "EngineCore.h"

namespace Toad
{

namespace Time
{
	ENGINE_API const sf::Time& GetDeltaTimeRaw();
	ENGINE_API float GetDeltaTime();
	ENGINE_API void SetFixedDeltaTime(float fdt);
	ENGINE_API float GetFixedDeltaTime();
    ENGINE_API float GetTime();
	ENGINE_API void SetTimeScale(float scale);
	ENGINE_API void UpdateDeltaTime();
};

}