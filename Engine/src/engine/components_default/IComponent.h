#pragma once

#include "EngineMeta.h"

namespace Toad
{

class Component
{
public:
	virtual void Start() = 0;
	virtual void Update() = 0;

	static void CallComponentsStart();
	static void CallComponentsUpdate();

private:
	inline static std::vector<Component> m_components = {};
};

}