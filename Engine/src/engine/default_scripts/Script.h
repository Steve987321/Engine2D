#pragma once

#include <EngineCore.h>

#include "engine/systems/Reflection.h"

#include <functional>

#if defined(TOAD_EDITOR) || !defined(NDEBUG)
#include "imgui/imgui.h"
#endif

#define EXPOSE_VAR(T) m_reflection.Add(#T, &(T))

#define SCRIPT_CONSTRUCT(T)											\
explicit T(std::string_view name) : Script(name)					\
{																	\
	m_name = name;													\
	ExposeVars();													\
}																	\
Script* Clone() override											\
{																	\
	T* pScript = new T(*this);										\
	pScript->ExposeVars();											\
	return dynamic_cast<Script*>(pScript);							\
}

namespace Toad
{
	class Object;

class ENGINE_API Script
{
public:
	Script(std::string_view name);

	const std::string& GetName() const;

	virtual void OnUpdate(Object* obj);
	virtual void OnFixedUpdate(Object* obj);
	virtual void OnLateUpdate(Object* obj);
	virtual void OnStart(Object* obj);
	virtual void OnStop(Object* obj);
	virtual void OnRender(Object* obj, sf::RenderTarget& target);
#ifdef TOAD_EDITOR
	virtual void OnEditorUI(Object* obj, ImGuiContext* ctx);
#endif 
#if defined(TOAD_EDITOR) || !defined(NDEBUG)
	virtual void OnImGui(Object* obj, ImGuiContext* ctx);
#endif 
	virtual void ExposeVars();

	virtual Script* Clone() = 0;
 
	Reflection& GetReflection();
	
protected:
	std::string m_name;
	Reflection m_reflection;
};

}
