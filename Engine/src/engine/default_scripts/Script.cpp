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

void Script::OnRender(Object* obj, sf::RenderTarget& target)
{
}

#ifdef TOAD_EDITOR
void Script::OnEditorUI(Object* obj, const UICtx& ctx)
{
}
#endif

#ifdef USE_IMGUI
void Script::OnImGui(Object* obj, const UICtx& ctx)
{
}
#endif

void Script::OnEnd(Object* obj, Scene* next_scene)
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
