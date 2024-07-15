#include "pch.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "Engine/Engine.h"

#include "FSMStateNode.h"
#include "../FSMGraphEditor.h"

#include "engine/utils/FSM.h"
#include "../UI.h"
#include "imgui-SFML.h"

namespace Toad
{

	FSMStateNode::FSMStateNode(State& state)
		: _state(state)
	{

	}

	void FSMStateNode::Show()
	{

	}

}