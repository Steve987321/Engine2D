#pragma once

#include <EngineCore.h>
#include "nlohmann/json.hpp"
#include "engine/Types.h"

namespace Toad
{

	using json = nlohmann::json;

	//using FAnimTransition = std::function<void(int)>;

struct AnimationFrame
{
	Vec2f position{};
	Vec2f scale{};
	float rotation = 0; 

	// ui 
	bool is_key = false;
};

struct Animation
{
	std::vector<AnimationFrame> frames;
	//std::vector<FAnimTransition> frames;

	//json Serialize() const;
};

//void AnimationDeserialize(const json& data);

}