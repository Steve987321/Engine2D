#pragma once

#include <EngineCore.h>

#define SCRIPT_CONSTRUCT(T) NULL

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

protected:
	std::string m_name;
};

}
