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
	State(std::string_view name);

public:
	virtual void OnEnter() {};
	virtual void OnExit() {};

public:
	std::string name;
	std::vector<Transition> transitions;

};

template<typename T> 
class TransitionCondition
{
public:
	TransitionCondition(T& a, T& b);

public:
	bool IsConditionMet() const 
	{
		switch (comparison_type)
		{
		case CompareType::NOTEQUAL:
			return m_a != m_b;
		case CompareType::EQUAL:
			return m_a == m_b;
		case CompareType::EQUALGREATERTHAN:
			return m_a >= m_b;
		case CompareType::GREATERTHAN:
			return m_a > m_b;
		case CompareType::EQUALLESSTHAN:
			return m_a <= m_b;
		case CompareType::LESSTHAN:
			return m_a < m_b;
		default:
			return false;
		}
	}

private:
	enum class CompareType
	{
		NOTEQUAL,			// a!=b
		EQUAL,				// a==b	
		EQUALGREATERTHAN,	// a>=b
		GREATERTHAN,		// a>b
		EQUALLESSTHAN,		// a<=b
		LESSTHAN			// a<b
	};

	CompareType comparison_type;
	T& m_a;
	T& m_b;
};

class Transition
{
public:
	friend class FSM;

	virtual void Invoke();

	// checks the conditions if a transition should happen, should call Invoke() if this returns true.
	bool IsTransitionAllowed();

	std::vector<TransitionCondition<int>> conditions_i32;
	std::vector<TransitionCondition<float>> conditions_flt;
	std::vector<TransitionCondition<std::string>> conditions_str;

protected:
	State& m_prevState;
	State& m_nextState;

};

/// Handle a state machine with states and transitions
class FSM
{
public:
	FSM();
	~FSM();

	void Update();

	void AddState(State state);

	void AddVariable(std::string_view name, int var);
	void AddVariable(std::string_view name, float var);
	void AddVariable(std::string_view name, std::string var);

	std::vector<std::pair<std::string, int>> varsi{};
	std::vector<std::pair<std::string, float>> varsflt{};
	std::vector<std::pair<std::string, std::string>> varsstr{};
public:
	json Serialize() const;

private:
	std::vector<State> m_states{};


	State* m_currentState = nullptr;
};

}