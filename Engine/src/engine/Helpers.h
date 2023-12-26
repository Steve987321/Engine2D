#pragma once

#include <SFML/Graphics/Color.hpp>
#include <imgui/imgui.h>

#include "EngineCore.h"

namespace Toad
{
	/// @brief fast random number generator with bounds
	/// @return fast int in range (inclusive) 
	ENGINE_API int rand_int(int min, int max);
	
	/// @brief fast random number generator with bounds
	/// @return fast float in range (inclusive) 
	ENGINE_API float rand_float(float min, float max);

	/// Returns the date as a string with the given format
	ENGINE_API std::string get_date_str(std::string_view format);
}
