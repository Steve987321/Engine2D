#include "pch.h"
#include "imgui.h"
#include "AnimationController.h"

namespace Toad
{

void AnimationController::Play()
{
	m_isPlaying = true;
}

void AnimationController::Stop()
{
	m_isPlaying = false;
}

void AnimationController::OnStart(Object* obj)
{
	
}

void AnimationController::OnUpdate(Object* obj)
{
	// #TODO tie to the animation frames rate 

	if (m_isPlaying)
	{

	}
}

void AnimationController::OnEditorUI(Object* obj)
{
	Script::OnEditorUI(obj);
	if (m_attachedAnimation)
	{
		ImGui::Text(m_attachedAnimation->name.c_str());
		ImGui::Text("is playing: %s", m_isPlaying ? "true" : "false");
	}
	else
	{
		ImGui::TextColored({ 1, 1, 0, 1 }, "No animation is attached to this controller");
	}
}

}