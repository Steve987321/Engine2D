#include "pch.h"
#include "imgui.h"
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

void AnimationController::OnEditorUI(Object* obj)
{
	Script::OnEditorUI(obj);

	ImGui::Text(" Hello from script Animation Controller");
}

}