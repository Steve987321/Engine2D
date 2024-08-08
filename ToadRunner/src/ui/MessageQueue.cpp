#include "pch.h"
#include "engine/Engine.h"

#include "UIHelpers.h"

#include "MessageQueue.h"

namespace Toad
{

	void MessageQueue::AddToMessageQueue(MessageQueueMessage msg)
	{
		m_messageQueue.emplace_back(msg, Toad::Timer{true});
	}

	void MessageQueue::Show()
	{
		const ImVec2 prev_cursor_pos = ImGui::GetCursorPos();
		ImVec2 starting_pos;
		if (m_placement == MessageQueuePlacement::RIGHT)
		{
			starting_pos = ImGui::GetContentRegionAvail();
			starting_pos.x -= m_size.x;
			starting_pos.y += 50.f;
		}
		
		constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings;
		uint32_t i = 0;
		for (auto& [message, timer] : m_messageQueue)
		{
			ImVec2 pos = starting_pos;
			pos.y += i * (m_size.y + 10.f);
			ImGui::SetNextWindowPos(pos);
			ImGui::PushID(i);
			ImGui::SetNextWindowSize(m_size);
			ImGui::Begin(std::to_string(i).c_str(), nullptr, window_flags);
			
			ImGui::Text("%s", message.title.c_str());
			ImGui::Separator();
			ImGui::Text("%s", message.message.c_str());
			ImGui::End();

			ImGui::PopID();

			i++;
		}
		auto it = m_messageQueue.begin();
		while (it != m_messageQueue.end())
		{
			if (it->second.Elapsed<>() > it->first.show_time_ms)
				it = m_messageQueue.erase(it);
			else 
				it++;
		}
		ImGui::SetCursorPos(prev_cursor_pos);
	}
}