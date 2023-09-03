#pragma once

#include "EngineCore.h"
#include "EngineMeta.h"

#include "../ScriptBase.h"

class Object : public ScriptBase, public sf::Transformable
{
public:
	Object();

	const char* GetName() const;

private:
	const char* m_name;
};

