#pragma once

#include <EngineCore.h>

#define SCRIPTBASE(T) NULL

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

private:
	std::string m_name;
};

}
