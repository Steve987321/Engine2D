#pragma once

#include "EngineCore.h"

#include "engine/systems/Reflection.h"

#include <functional>

#ifdef USE_IMGUI
#include "imgui/imgui.h"
#include "implot/implot.h"

struct UICtx
{
    ImGuiContext* imgui_ctx = nullptr;
    ImPlotContext* implot_ctx = nullptr;
};
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
	T* script = new T(*this);										\
	script->ExposeVars();											\
	return dynamic_cast<Script*>(script);							\
}

#define UI_APPLY_CTX(ctx)                   \
ImGui::SetCurrentContext(ctx.imgui_ctx);    \
ImPlot::SetCurrentContext(ctx.implot_ctx)   

namespace Toad
{
	class Object;
	class Scene;

class ENGINE_API Script
{
public:
	Script(std::string_view name);

	const std::string& GetName() const;
	
	virtual void OnUpdate(Object* obj);
	virtual void OnFixedUpdate(Object* obj);
	virtual void OnLateUpdate(Object* obj);
	virtual void OnStart(Object* obj);
	virtual void OnRender(Object* obj, sf::RenderTarget& target);
#ifdef TOAD_EDITOR
	virtual void OnEditorUI(Object* obj, const UICtx& ctx);
#endif 
#ifdef USE_IMGUI
	virtual void OnImGui(Object* obj, const UICtx& ctx);
#endif 

	virtual void OnEnd(Object* obj, Scene* next_scene);

	// exposes vars to inspector and scene file
	virtual void ExposeVars();

	virtual Script* Clone() = 0;
 
	Reflection& GetReflection();
	
protected:
	std::string m_name;
	Reflection m_reflection;
};

}
