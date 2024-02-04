#include "pch.h"
#include "AnimationController.h"

namespace Toad
{

void AnimationController::Play()
{
}

void AnimationController::Stop()
{
}

void AnimationController::OnStart(Object* obj)
{
}

void AnimationController::OnUpdate(Object* obj)
{
}

void AnimationController::ExposeVars()
{
	EXPOSE_VAR(m_usedAnimId);
}

}