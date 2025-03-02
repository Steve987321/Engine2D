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

		// placement corner 
		ImVec2 starting_pos;
		if (m_placement == MessageQueuePlacement::RIGHT)
		{
			starting_pos = ImGui::GetContentRegionAvail();
			starting_pos.y += 50.f;
		}
		
		constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings;
		uint32_t i = 0;
		ImDrawList* draw = ImGui::GetForegroundDrawList();

		for (auto& [message, timer] : m_messageQueue)
		{
			ImFont* font = ImGui::GetDefaultFont();
			const float font_size = 23;
			ImVec2 pos = starting_pos;
			ImVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, -1, message.msg.c_str());
			pos.x -= text_size.x;
			pos.x -= 100.f;
			pos.y += i * (text_size.y + 5.f);

			float a = (1.f + ImSin((float)ImGui::GetTime() * 10)) / 2.f;
			// draw bg 
			draw->AddRectFilled(pos, { pos.x + text_size.x, pos.y + text_size.y }, IM_COL32(10, 10, 10, 200));

			switch (message.category)
			{

			case MessageCategory::ENGINE:
			{
				draw->AddText(font, font_size, pos, IM_COL32(0, ImLerp(150, 255, a), 0, 255), message.msg.c_str());
				break;
			}
			case MessageCategory::GAME:
			{
				uint8_t rg = ImLerp(200, 255, a);
				draw->AddText(font, font_size, pos, IM_COL32(rg, rg, 0, 255), message.msg.c_str());
				break;
			}
			case MessageCategory::UI:
			{
				uint8_t b = ImLerp(200, 255, a);
				draw->AddText(font, font_size, pos, IM_COL32(0, b, 0, 255), message.msg.c_str());
				break;
			}
			case MessageCategory::OTHER:
			default:
			{
				break;
			}

			}

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