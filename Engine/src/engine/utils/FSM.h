#pragma once

#include "nlohmann/json.hpp"

namespace Toad
{

using json = nlohmann::json;

class State;
class Transition;
class FSM;

class State
{
public:
	State();

public:

private:
	std::vector<Transition> m_transitions;
};

class Transition
{
public:
	void Invoke();

private:
	State& m_prevState;
	State& m_nextState;
};

/// Handle a state machine with states and transitions
class FSM
{
public:
	FSM();
	~FSM();

public:
	json Serialize() const;
private:
	std::vector<State> m_states;
};

}