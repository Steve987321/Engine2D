#pragma once

#include <SFML/Graphics/Color.hpp>
#include <imgui/imgui.h>

#include "EngineCore.h"
#include "engine/Types.h"
#include "nlohmann/json.hpp"
#include "engine/FormatStr.h"

// will do nothing and log an error if key isn't in data else will set data to data[key]
#define GET_JSON_ELEMENT(val, data, key) if (data.contains(key)) val = data[key]; else LOGERRORF("[{}] Failed to load property: {}", __FUNCTION__, key);

#ifndef DEFINE_ENUM_FLAG_OPERATORS
#define DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE) \
extern "C++" { \
inline constexpr ENUMTYPE operator | (ENUMTYPE a, ENUMTYPE b) noexcept { return ENUMTYPE(((int)a) | ((int)b)); } \
inline ENUMTYPE &operator |= (ENUMTYPE &a, ENUMTYPE b) noexcept { return (ENUMTYPE &)(((int &)a) |= ((int)b)); } \
inline constexpr ENUMTYPE operator & (ENUMTYPE a, ENUMTYPE b) noexcept { return ENUMTYPE(((int)a) & ((int)b)); } \
inline ENUMTYPE &operator &= (ENUMTYPE &a, ENUMTYPE b) noexcept { return (ENUMTYPE &)(((int &)a) &= ((int)b)); } \
inline constexpr ENUMTYPE operator ~ (ENUMTYPE a) noexcept { return ENUMTYPE(~((int)a)); } \
inline constexpr ENUMTYPE operator ^ (ENUMTYPE a, ENUMTYPE b) noexcept { return ENUMTYPE(((int)a) ^ ((int)b)); } \
inline ENUMTYPE &operator ^= (ENUMTYPE &a, ENUMTYPE b) noexcept { return (ENUMTYPE &)(((int &)a) ^= ((int)b)); } \
}
// from winnt.h
#endif

namespace Toad
{
	class Script;
	class Object;

	/// @brief fast random number generator with bounds
	/// @return fast int in range (inclusive) 
	ENGINE_API int rand_int(int min, int max);
	
	/// @brief fast random number generator with bounds
	/// @return fast float in range (inclusive) 
	ENGINE_API float rand_float(float min, float max);

	/// Returns the date as a string with the given format
	ENGINE_API std::string get_date_str(std::string_view format);

	ENGINE_API std::filesystem::path get_exe_path();

	ENGINE_API float distance(const Vec2f& a, const Vec2f& b);

	ENGINE_API float deg_to_rad(float degrees);

	ENGINE_API float rad_to_deg(float radians);

	template<typename T>
	ENGINE_API inline bool get_json_element(T& val, const nlohmann::ordered_json& data, std::string_view key, std::string error_msg = "") noexcept
	{
		if (data.contains(key) && !data.at(key).is_null())
		{
			try
			{
				val = data.at(key).get<T>();
				return true;
			}
			catch (const nlohmann::json::exception& e)
			{
				error_msg = format_str("[JSON] Failed to load property {}, {}", key.data(), e.what());
				return false;
			}
		}
		else
		{
			error_msg = format_str("[JSON] Failed to load property: {}, doesn't exist", key.data());
			return false;
		}
	}

	ENGINE_API std::string GetFileContents(const char* file);

	template<class T>
	T* get_object_as_type(Object* obj)
	{
		return dynamic_cast<T*>(obj);
	}

	template<class T>
	T* get_script_as_type(Script* script)
	{
		return dynamic_cast<T*>(script);
	}
}
