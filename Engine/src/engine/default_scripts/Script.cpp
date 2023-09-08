#include "pch.h"

#include "EngineCore.h"
#include "EngineMeta.h"

#include "Script.h"

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

void Script::OnUpdate()
{
}

void Script::OnCreate()
{
}

void Script::OnDestroy()
{
}

}