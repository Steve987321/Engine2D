#pragma once

#include <EngineCore.h>

#include "engine/systems/Reflection.h"

#include <functional>

#define EXPOSE_VAR(T) m_reflection.Add(#T, &(T))

#define SCRIPT_CONSTRUCT(T)											\
T(std::string_view name) : Script(name)								\
{																	\
	m_name = name;													\
	ExposeVars();													\
}																	\
std::shared_ptr<Script> Clone() override							\
{																	\
	auto pScript = new T(*this);									\
	pScript->ExposeVars();											\
	return std::shared_ptr<Script>(dynamic_cast<Script*>(pScript));	\
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
	virtual void OnStart(Object* obj);
	virtual void OnDestroy(Object* obj);
	virtual void OnEditorUI(Object* obj);

	virtual void ExposeVars();

	virtual std::shared_ptr<Script> Clone() = 0;

	Reflection& GetReflection();
	
protected:
	std::string m_name;
	Reflection m_reflection;
};

}
