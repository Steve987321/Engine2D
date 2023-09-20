#pragma once

#include <EngineCore.h>

#include "engine/systems/Reflection.h"

#include <functional>

#ifdef _DEBUG
#define EXPOSE_VAR(T) m_reflection.Add(#T, &(T))
#else
#define EXPOSE_VAR(T) 0
#endif

#define SCRIPT_CONSTRUCT(T)				\
T(std::string_view name) : Script(name)	\
{										\
	m_name = name;						\
	ExposeVars();						\
}

#define EXPOSABLE static inline 

namespace Toad
{
	class Object;

class ENGINE_API Script
{
public:
	Script(std::string_view name);
	
	const std::string& GetName() const;

	virtual void OnUpdate(Object* obj);
	virtual void OnCreate(Object* obj);
	virtual void OnDestroy(Object* obj);

	virtual void ExposeVars();

	Reflection& GetReflection();
	
protected:
	std::string m_name;
	Reflection m_reflection;
};

}
