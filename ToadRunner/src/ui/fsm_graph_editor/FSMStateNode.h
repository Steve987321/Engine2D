#pragma once

namespace Toad
{

class FSMStateNode
{
public:
	FSMStateNode(State& state);

public:
	ImVec2 Pos{};

	void Show();

private:
	State& _state;
};

}