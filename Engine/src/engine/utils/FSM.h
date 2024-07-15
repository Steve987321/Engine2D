#pragma once

#include "EngineCore.h"
#include "nlohmann/json.hpp"
#include "engine/utils/Helpers.h"

namespace Toad
{

using json = nlohmann::json;

class State;
class Transition;
class FSM;

enum class CompareType
{
	NOTEQUAL,			// a!=b
	EQUAL,				// a==b	
	EQUALGREATERTHAN,	// a>=b
	GREATERTHAN,		// a>b
	EQUALLESSTHAN,		// a<=b
	LESSTHAN			// a<b
};

enum class FSMVariableType
{
	FLOAT,
	INT32
};

template<typename T>
struct ENGINE_API FSMVariable
{
	std::string name;
	T data{};
};

class ENGINE_API State
{
public:
	explicit State(std::string_view name);
	State() = default;

public:
	std::string name;
	std::vector<Transition> transitions{};

	virtual void OnEnter() {};
	virtual void OnExit() {};
};

class ENGINE_API TransitionCondition
{
public:
	TransitionCondition(FSM* fsm, int a, int b, FSMVariableType var_type, CompareType compare_type)
		: m_fsm(fsm), m_a(a), m_b(b), m_varType(var_type), m_comparisonType(compare_type)
	{}

	TransitionCondition& operator=(const TransitionCondition& other);
public:
	json Serialize() const;


	static TransitionCondition Deserialize(const json& data, FSM& fsm);

	bool IsConditionMet() const;

private:
	template<typename T>
	bool CompareAB(FSMVariable<T>& a, FSMVariable<T>& b) const 
	{
		switch (m_comparisonType)
		{
		case CompareType::NOTEQUAL:
			return a.data != b.data;
		case CompareType::EQUAL:
			return a.data == b.data;
		case CompareType::EQUALGREATERTHAN:
			return a.data >= b.data;
		case CompareType::GREATERTHAN:
			return a.data > b.data;
		case CompareType::EQUALLESSTHAN:
			return a.data <= b.data;
		case CompareType::LESSTHAN:
			return a.data < b.data;
		default:
			return false;
		}
	}

private:
	CompareType m_comparisonType;
	int m_a = 0;
	int m_b = 0;
	FSMVariableType m_varType;

	FSM* m_fsm;
};

class ENGINE_API Transition
{
public:
	Transition(FSM& fsm, int prev, int next)
		: m_fsm(fsm), m_prevStateIndex(prev), m_nextStateIndex(next)
	{}

	Transition& operator=(const Transition& other);

	friend class FSM;
	
	// gets called after State.Exit
	virtual void Invoke();

	// checks the conditions if a transition should happen, should call Invoke() if this returns true.
	bool IsTransitionAllowed();

	void AddConditionI32(const TransitionCondition& condition_i32);
	void AddConditionFlt(const TransitionCondition& condition_i32);

	State* GetPreviousState();
	State* GetNextState();

private:
	int m_prevStateIndex;
	int m_nextStateIndex;
	FSM& m_fsm;

private:
	std::vector<TransitionCondition> conditions_i32{};
	std::vector<TransitionCondition> conditions_flt{};
};

/// Handle a state machine with states and transitions
class ENGINE_API FSM
{
public:
	explicit FSM(std::string_view name);
	FSM();
	~FSM();

	void Update();

	State* GetCurrentState() const;
	const std::string& GetName() const;
	std::vector<State>& GetStates();

	// can return nullptr
	State* GetStateByName(std::string_view name);

	void AddState(State state);
	void AddVariable(std::string_view name, int var);
	void AddVariable(std::string_view name, float var);

	std::vector<FSMVariable<int>> varsi32{};
	std::vector<FSMVariable<float>> varsflt{};
	
public:
	json Serialize() const;
	static FSM Deserialize(const json& data);

private:
	std::vector<State> m_states{};
	std::string m_name;
	State* m_currentState = nullptr;
};

}