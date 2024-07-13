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
	virtual void OnEnter() {};
	virtual void OnExit() {};

public:
	std::string name;
	std::vector<Transition> transitions{};
};

template<typename T> 
class ENGINE_API TransitionCondition
{
public:
	TransitionCondition(FSMVariable<T>& a, FSMVariable<T>& b, CompareType type)
		: m_a(a), m_b(b), m_comparisonType(type)
	{
	}

public:
	json Serialize() const
	{
		json data;
		data["a"] = m_a.name;
		data["b"] = m_b.name;
		data["compare"] = m_comparisonType;
		return data;
	}

	TransitionCondition<T>& operator=(const TransitionCondition<T>& other)
	{
		if (this != &other)
		{
			m_a = other.m_a;
			m_b = other.m_b;
			m_comparisonType = other.m_comparisonType;
		}
		return *this;
	}

	static TransitionCondition<T> Deserialize(const json& data, std::vector<FSMVariable<T>>& variables)
	{
		std::string a_name;
		std::string b_name;
		get_json_element(a_name, data, "a");
		get_json_element(b_name, data, "b");
		CompareType compare_type = CompareType::EQUAL;
		get_json_element(compare_type, data, "compare");

		FSMVariable<T>* a_var = nullptr;
		FSMVariable<T>* b_var = nullptr;
		for (auto& var : variables)
		{
			if (var.name == a_name)
				a_var = &var;
			else if (var.name == b_name)
				b_var = &var;

			if (a_var && b_var)
				break;
		}

		if (!a_var || !b_var)
		{
			throw std::runtime_error("a_var or b_var is nullptr");
		}

		return TransitionCondition<T>(*a_var, *b_var, compare_type);
	}

	bool IsConditionMet() const 
	{
		switch (m_comparisonType)
		{
		case CompareType::NOTEQUAL:
			return m_a.data != m_b.data;
		case CompareType::EQUAL:
			return m_a.data == m_b.data;
		case CompareType::EQUALGREATERTHAN:
			return m_a.data >= m_b.data;
		case CompareType::GREATERTHAN:
			return m_a.data > m_b.data;
		case CompareType::EQUALLESSTHAN:
			return m_a.data <= m_b.data;
		case CompareType::LESSTHAN:
			return m_a.data < m_b.data;
		default:
			return false;
		}
	}

private:
	CompareType m_comparisonType;
	FSMVariable<T>& m_a;
	FSMVariable<T>& m_b;
};

class ENGINE_API Transition
{
public:
	Transition(State& prev, State& next)
		: m_prevState(prev), m_nextState(next)
	{}

	Transition& operator=(const Transition& other);

	friend class FSM;
	
	// gets called after State.Exit
	virtual void Invoke();

	// checks the conditions if a transition should happen, should call Invoke() if this returns true.
	bool IsTransitionAllowed();

	void AddCondition(const TransitionCondition<int>& condition_i32);
	void AddCondition(const TransitionCondition<float>& condition_i32);

private:
	State& m_prevState;
	State& m_nextState;

	std::vector<TransitionCondition<int>> conditions_i32{};
	std::vector<TransitionCondition<float>> conditions_flt{};
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