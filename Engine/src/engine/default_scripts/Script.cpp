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

void Script::OnFixedUpdate(Object* obj)
{

}

void Script::OnLateUpdate(Object* obj)
{
}

void Script::OnStart(Object* obj)
{
}

void Script::OnStop(Object* obj)
{
}

void Script::OnRender(Object* obj, sf::RenderTarget& target)
{
}

#ifdef TOAD_EDITOR
void Script::OnEditorUI(Object* obj, ImGuiContext* ctx)
{
}
#endif

#if defined(TOAD_EDITOR) || !defined(NDEBUG)
void Script::OnImGui(Object* obj, ImGuiContext* ctx)
{
}
#endif

void Script::ExposeVars()
{
}

Reflection& Script::GetReflection()
{
	return m_reflection;
}

}
