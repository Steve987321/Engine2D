#pragma once

#include <EngineCore.h>
#include "nlohmann/json.hpp"
#include "engine/Types.h"

namespace Toad
{

using json = nlohmann::json;

//using FAnimTransition = std::function<void(int)>;

enum class AnimationForType
{
	SPRITE,
	CIRCLE,
	CAMERA,
	AUDIO,
	UITEXT,
};

struct AnimationFrame
{
	Vec2f position{};
	Vec2f scale{};
	float rotation = 0; 

	std::pair<std::string, std::string> script_prop;
	std::pair<std::string, std::string> obj_prop;

	// ui 
	bool is_key = false;
};

struct Animation
{
	std::string name = "unnamed";
	std::vector<AnimationFrame> frames{};
	//std::vector<FAnimTransition> frames;

	int frame_rate = 5;
	int frame_length = 0; // should be same as frames.size()
	bool is_looping = false;

	json Serialize() const;
};

//void AnimationDeserialize(const json& data);

}