#pragma once

#ifdef ERROR
#undef ERROR
#endif 

#include "engine/systems/Timer.h"

struct ImVec2;

namespace Toad
{

enum class MessageType
{
	ERROR,
	WARNING,
	INFO
};

enum class MessageCategory
{
	OTHER,
	ENGINE,
	GAME,
	UI,
};

enum class MessageQueuePlacement
{
	LEFT,
	RIGHT
};

struct MessageQueueMessage
{
	std::string title;
	std::string message;
	float show_time_ms = 1000.f;
	MessageType type = MessageType::INFO;
	MessageCategory category = MessageCategory::OTHER;
};


// vertical imgui message queue
class MessageQueue
{
public:
	MessageQueue(ImVec2 size, MessageQueuePlacement placement)
		: m_size(size), m_placement(placement)
	{}

public:
	void AddToMessageQueue(MessageQueueMessage msg_type);
	void Show();

private:
	std::vector<std::pair<MessageQueueMessage, Timer>> m_messageQueue{};
	ImVec2 m_size{};
	MessageQueuePlacement m_placement = MessageQueuePlacement::RIGHT;
};

}
