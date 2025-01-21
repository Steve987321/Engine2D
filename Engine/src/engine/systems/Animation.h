#pragma once

#include <EngineCore.h>
#include "nlohmann/json.hpp"
#include "engine/Types.h"

namespace Toad
{

using json = nlohmann::ordered_json;

//using FAnimTransition = std::function<void(int)>;

struct AnimationFrame
{
	Vec2f position{};
	Vec2f scale{};
	float rotation = 0; 

	Vec2i texture_pos;
	IntRect texture_rect;

	// scripts with name and value 
	std::vector<std::pair<std::string, json>> scripts;
};

struct Animation
{
	std::string name = "unnamed";

	// key frames 
	std::vector<AnimationFrame> frames{};

	int frame_rate = 5;
	bool is_looping = false;

	json Serialize() const;
};

//void AnimationDeserialize(const json& data);

}