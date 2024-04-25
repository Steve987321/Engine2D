#pragma once

// reflection for variables in scripts
// should be used on script class member variables

// this will be used for the editor, and scenes to serialize variables

// bool, int8, int16, int32, float, string

#include "EngineCore.h"

namespace Toad
{

// non pointer values 
struct ENGINE_API ReflectVarsCopy
{
	std::unordered_map<std::string, bool>			b = {};
	std::unordered_map<std::string, int8_t>			i8 = {};
	std::unordered_map<std::string, int16_t>		i16 = {};
	std::unordered_map<std::string, int32_t>		i32 = {};
	std::unordered_map<std::string, float>			flt = {};
	std::unordered_map<std::string, std::string>	str = {};

	void copy(ReflectVarsCopy& dst)
	{
		for (const auto& [n, v] : b)
			dst.b[n] = v;
		for (const auto& [n, v] : i8)
			dst.i8[n] = v;
		for (const auto& [n, v] : i16)
			dst.i16[n] = v;
		for (const auto& [n, v] : i32)
			dst.i32[n] = v;
		for (const auto& [n, v] : flt)
			dst.flt[n] = v;
		for (const auto& [n, v] : str)
			dst.str[n] = v;
	}
};

struct ENGINE_API ReflectVars
{
	std::unordered_map<std::string, bool*>			b = {};
	std::unordered_map<std::string, int8_t*>		i8 = {};
	std::unordered_map<std::string, int16_t*>		i16 = {};
	std::unordered_map<std::string, int32_t*>		i32 = {};
	std::unordered_map<std::string, float*>			flt = {};
	std::unordered_map<std::string, std::string*>	str = {};

	void copy(ReflectVarsCopy& dst)
	{
		for (const auto& [n, v] : b)
			dst.b[n] = *v;
		for (const auto& [n, v] : i8)
			dst.i8[n] = *v;
		for (const auto& [n, v] : i16)
			dst.i16[n] = *v;
		for (const auto& [n, v] : i32)
			dst.i32[n] = *v;
		for (const auto& [n, v] : flt)
			dst.flt[n] = *v;
		for (const auto& [n, v] : str)
			dst.str[n] = *v;
	}
};

struct ENGINE_API Reflection
{
	ReflectVars& Get();

	void Add(std::string_view name, bool* val);
	void Add(std::string_view name, std::int8_t* val);
	void Add(std::string_view name, std::int16_t* val);
	void Add(std::string_view name, std::int32_t* val);
	void Add(std::string_view name, float* val);
	void Add(std::string_view name, std::string* val);

	ReflectVars vars;
};

}