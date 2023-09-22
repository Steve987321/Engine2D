#include "pch.h"
#include "Reflection.h"

namespace Toad
{

ReflectVars& Reflection::Get()
{
	return vars;
}

void Reflection::Add(std::string_view name, bool* val)
{
	vars.b[name.data()] = val;
}

void Reflection::Add(std::string_view name, std::int8_t* val)
{
	vars.i8[name.data()] = val;
}

void Reflection::Add(std::string_view name, std::int16_t* val)
{
	vars.i16[name.data()] = val;
}

void Reflection::Add(std::string_view name, std::int32_t* val)
{
	vars.i32[name.data()] = val;
}

void Reflection::Add(std::string_view name, float* val)
{
	vars.flt[name.data()] = val;
}

void Reflection::Add(std::string_view name, std::string* val)
{
	vars.str[name.data()] = val;
}

}