#include "pch.h"
#include "imgui.h"
#include "AnimationController.h"

#include <engine/Engine.h>

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

void AnimationController::OnLateUpdate(Object* obj)
{
	// #TODO tie to the animation frames rate 

	if (Engine::Get().GetObjectAsType<Toad::Sprite>(obj))

	if (m_isPlaying)
	{
		if (m_attachedAnimation)
		{
			for (const auto& frame : m_attachedAnimation->frames)
			{
				//obj->
			}
		}
	}
}

void AnimationController::OnEditorUI(Object* obj, ImGuiContext* ctx)
{
	Script::OnEditorUI(obj, ctx);
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