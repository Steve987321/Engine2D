#include "pch.h"
#include "Script.h"

#include "EngineCore.h"
#include "EngineMeta.h"

namespace Toad
{

Script::Script(std::string_view name)
{
	m_name = name;
}

const std::string& Script::GetName() const
{
	return m_name;
}

void Script::OnUpdate(Object* obj)
{
}

void Script::OnStart(Object* obj)
{
}

void Script::OnDestroy(Object* obj)
{
}

void Script::ExposeVars()
{
}

Reflection& Script::GetReflection()
{
	return m_reflection;
}

}
