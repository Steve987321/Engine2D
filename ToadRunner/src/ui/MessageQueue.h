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
	int showing_duration_sec = 1;
	MessageType type = MessageType::INFO;
	MessageCategory category = MessageCategory::OTHER;
};

// vertical imgui message queue
class MessageQueue
{
public:
	MessageQueue(ImVec2 area_region, MessageQueuePlacement placement)
		: _region(area_region), _placement(placement)
	{}

public:
	void AddToMessageQueue(MessageQueueMessage msg_type);
	void Show();

private:
	std::queue<std::pair<MessageQueueMessage, Timer>> _messageQueue{};
	ImVec2 _region{};
	MessageQueuePlacement _placement = MessageQueuePlacement::RIGHT;
};

}
