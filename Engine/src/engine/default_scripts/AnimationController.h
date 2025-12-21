#pragma once

#include "Script.h"
#include "engine/systems/Animation.h"
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
	void OnLateUpdate(Object* obj) override;
#ifdef TOAD_EDITOR
	void OnEditorUI(Object* obj, const UICtx& ctx) override;
#endif
private: 
	Animation* m_attachedAnimation = nullptr;
	bool m_isPlaying = false;
};

}
