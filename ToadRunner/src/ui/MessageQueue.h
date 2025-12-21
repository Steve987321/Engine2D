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
	std::string msg;
	float show_time_ms = 1000.f;
	MessageType type = MessageType::INFO;
	MessageCategory category = MessageCategory::OTHER;
    bool permanent = false;
};


// vertical imgui message queue
namespace MessageQueue
{
    // Return id of message 
	size_t AddToMessageQueue(MessageQueueMessage msg_type);
    void RemoveMessage(size_t id);

	void Show();
};

}
