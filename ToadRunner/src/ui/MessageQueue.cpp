#include "pch.h"

#define IMGUI_DEFINE_MATH_OPERATORS

#include "engine/Engine.h"

#include "UIHelpers.h"

#include "MessageQueue.h"

namespace Toad
{

	static std::vector<std::pair<MessageQueueMessage, Timer>> message_queue{};
	static MessageQueuePlacement placement = MessageQueuePlacement::RIGHT;

	size_t MessageQueue::AddToMessageQueue(MessageQueueMessage msg)
	{
		message_queue.emplace_back(msg, Toad::Timer{true});
        return message_queue.size() - 1;
	}

    void MessageQueue::RemoveMessage(size_t id)
    {
        if (id < message_queue.size())
            message_queue.erase(message_queue.begin() + id);
    }

    void MessageQueue::Show()
	{
		const ImVec2 prev_cursor_pos = ImGui::GetCursorPos();

		// placement corner 
		ImVec2 starting_pos;
		if (placement == MessageQueuePlacement::RIGHT)
		{
			starting_pos = ImGui::GetContentRegionAvail();
			starting_pos.y += 50.f;
		}

		uint32_t i = 0;
		ImDrawList* draw = ImGui::GetForegroundDrawList();

		for (const auto& [message, timer] : message_queue)
		{
			ImFont* font = ImGui::GetDefaultFont();
			const float font_size = 15;
            const float margin = 10.f;
            const float text_spacing = 5.f;
            const float circle_status_size = 5.f;
			ImVec2 pos = starting_pos;
			ImVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, -1, message.msg.c_str());
			pos.x -= text_size.x + margin;
			pos.x -= 100.f;
			pos.y += i * (text_size.y + 10.f + margin);
            ImVec2 pos_end = { pos.x + text_size.x + margin, pos.y + text_size.y};
			float a = (1.f + ImSin((float)ImGui::GetTime() * 10)) / 2.f;
			// draw bg 
			draw->AddRectFilled(pos + ImVec2{-circle_status_size - 1, -circle_status_size * 2 - 1}, pos_end + ImVec2{1, 1}, IM_COL32(30, 30, 30, 255));
			draw->AddRectFilled(pos + ImVec2{-circle_status_size, -circle_status_size * 2}, pos_end, IM_COL32(20, 20, 20, 255));

            ImU32 circle_col = IM_COL32_WHITE;
            switch(message.type)
            {
                case MessageType::INFO:
                    circle_col = IM_COL32(255, 255, 255, 255);
                    break;
                case MessageType::WARNING:
                    circle_col = IM_COL32(255, 255, 0, 255);
                    break;
                case MessageType::ERROR:
                    circle_col = IM_COL32(255, 0, 0, 255);
                    break;
                default:
                    break;
            }
            
            draw->AddCircleFilled(pos + ImVec2{5,-5}, 5.f, circle_col);

            if (!message.permanent)
            {
                const float line_width = 3;

                pos_end.y += line_width;
                // draw timer 
                ImVec2 timer_pos_end = pos_end;
                float t = std::min(timer.Elapsed<>() / message.show_time_ms, 1.f);
                timer_pos_end.x = ImLerp(pos.x, pos_end.x, t);
                draw->AddRectFilled(ImVec2{pos.x, pos_end.y - line_width}, timer_pos_end, IM_COL32(75, 75, 255, 120));
            }

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
		auto it = message_queue.begin();
		while (it != message_queue.end())
		{
			if (!it->first.permanent && it->second.Elapsed<>() > it->first.show_time_ms)
				it = message_queue.erase(it);
			else 
				it++;
		}
		ImGui::SetCursorPos(prev_cursor_pos);
	}
}
