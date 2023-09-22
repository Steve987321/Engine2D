#pragma once

// reflection for variables in scripts
// variables need to be static for them to be allowed to be 'exposed'

// this will be used for the editor

// bool, int8, int16, int32, int64, uint32, uint64, float, string

#include "EngineCore.h"

namespace Toad
{

struct ENGINE_API ReflectVars
{
	std::unordered_map<std::string, bool*>			b = {};
	std::unordered_map<std::string, int8_t*>		i8 = {};
	std::unordered_map<std::string, int16_t*>		i16 = {};
	std::unordered_map<std::string, int32_t*>		i32 = {};
	//std::unordered_map<std::string, uint32_t*>	u32 = {};
	//std::unordered_map<std::string, uint64_t*>	u64 = {};
	std::unordered_map<std::string, float*>			flt = {};
	std::unordered_map<std::string, std::string*>	str = {};
};

struct ENGINE_API Reflection
{
	ReflectVars& Get();

	void Add(std::string_view name, bool* val);
	void Add(std::string_view name, std::int8_t* val);
	void Add(std::string_view name, std::int16_t* val);
	void Add(std::string_view name, std::int32_t* val);
	//void Add(std::string_view name, std::uint32_t* val);
	//void Add(std::string_view name, std::uint64_t* val);
	void Add(std::string_view name, float* val);
	void Add(std::string_view name, std::string* val);

	ReflectVars vars;
};

}