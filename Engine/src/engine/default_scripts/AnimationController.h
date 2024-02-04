#pragma once

#include "Script.h"
#include <EngineCore.h>

namespace Toad
{

class ENGINE_API AnimationController : public Script
{
public: 
	SCRIPT_CONSTRUCT(AnimationController)

public: 
	void Play();
	void Stop();

public:
	void OnStart(Object* obj) override;
	void OnUpdate(Object* obj) override;
	void ExposeVars() override;

private: 
	std::string m_usedAnimId = "";
};

}
