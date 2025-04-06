#pragma once

// can be used to handle a whole game using a simple graph 
// explanation (Game Flow System): 23/01/2025 02:09

namespace Toad
{
	class FlowNodeTransition;
	class FlowNode;
	class Flow;

// represents an arrow from node a to b
class FlowNodeTransition 
{
public:
	FlowNode* next;

	bool Condition();
};

class FlowNode
{
public:
	Flow& flow_ctx;

	FlowNodeTransition* transition = nullptr;

	std::function<void()> update;
};

class Flow
{
public:
    // #todo add ctx
    
	std::vector<FlowNode> nodes;
};

}
