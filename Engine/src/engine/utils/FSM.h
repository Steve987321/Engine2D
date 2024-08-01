#pragma once

#include "EngineCore.h"
#include "nlohmann/json.hpp"
#include "engine/utils/Helpers.h"
#include "engine/Logger.h"

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
	LESSTHAN,			// a<b
	COUNT				// Size
};

ENGINE_API std::string to_string(CompareType type);
ENGINE_API void to_string(CompareType type, char dest[3]);

enum class FSMVariableType
{
	FLOAT,
	INT32
};

template<typename T>
struct ENGINE_API FSMVariable
{
	FSMVariable(const std::string& name, const T& data)
		: name(name), data(data)
	{}

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

	std::function<void()> on_enter;
	std::function<void()> on_exit;
};

class ENGINE_API TransitionCondition
{
public:
	TransitionCondition(FSM* fsm, int a, int b, FSMVariableType var_type, CompareType compare_type)
		: m_fsm(fsm), a(a), b(b), m_varType(var_type), comparison_type(compare_type)
	{}

	TransitionCondition& operator=(const TransitionCondition& other);
public:
	int a = 0;
	int b = 0;
	CompareType comparison_type;

	json Serialize() const;

	static TransitionCondition Deserialize(const json& data, FSM& fsm);

	bool IsConditionMet() const;

private:
	template<typename T>
	bool CompareAB(FSMVariable<T>& a, FSMVariable<T>& b) const 
	{
		switch (comparison_type)
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
	FSMVariableType m_varType = FSMVariableType::INT32;
	FSM* m_fsm = nullptr;
};

class ENGINE_API Transition
{
public:
	Transition(FSM& fsm, int prev, int next)
		: m_fsm(fsm), m_prevStateIndex(prev), m_nextStateIndex(next)
	{}

	Transition& operator=(const Transition& other);

	friend class FSM;
	
	std::vector<TransitionCondition> conditions_i32{};
	std::vector<TransitionCondition> conditions_flt{};

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
};

/// Handle a state machine with states and transitions
class ENGINE_API FSM
{
public:
	explicit FSM(std::string_view name);
	FSM();
	~FSM();

	std::string name;

	void Update();

	// sets the initial state
	void Start();
	State* GetCurrentState() const;
	std::vector<State>& GetStates();

	// can return nullptr
	State* GetStateByName(std::string_view name);

	// don't add states while fsm is running
	// as this would cause an invalid current state
	void AddState(State state);

	template<typename T> 
	void AddVariable(std::string name, T var)
	{
		std::vector<FSMVariable<T>>* vars = GetVarsVec<T>();
		if (!vars)
		{
			LOGERROR("[FSM] Adding an unsupported variable type");
			return;
		}

		const std::string original_name = name;
		bool found = true;
		const auto fix_duplicate_name = [&]
			{
				found = false;
				for (int i = 0; i < vars->size(); i++)
				{
					if (name == (*vars)[i].name)
					{
						found = true;

						// fix it and break and check again
						while (name == (*vars)[i].name)
							name = original_name + '_' + std::to_string(i + 1);

						break;
					}
				}
			};

		while (found)
			fix_duplicate_name();

		vars->emplace_back(name, var);
	}

	template<typename T>
	void SetVariable(std::string_view name, T var)
	{
		std::vector<FSMVariable<T>>* vars = GetVarsVec<T>();

		if (!vars)
		{
			LOGERROR("[FSM] Setting an unsupported variable type");
			return;
		}
		
		for (FSMVariable<T>& fsm_var : *vars)
		{
			if (name == fsm_var.name)
			{
				fsm_var.data = var;
				break;
			}
		}
	}

	template<typename T>
	T GetVariable(std::string_view name)
	{
		std::vector<FSMVariable<T>>* vars = GetVarsVec<T>();
		if (!vars)
		{
			LOGERROR("[FSM] Setting an unsupported variable type");
			return;
		}

		for (FSMVariable<T>& fsm_var : *vars)
		{
			if (name == fsm_var.name)
				return fsm_var.data;
		}
	
		return 0;
	}

	std::vector<FSMVariable<int>> varsi32{};
	std::vector<FSMVariable<float>> varsflt{};
	
public:
	json Serialize() const;

	static FSM Deserialize(const json& data);
	static FSM Deserialize(const std::filesystem::path& file);

private:
	std::vector<State> m_states{};
	State* m_currentState = nullptr;

	template<typename T>
	std::vector<FSMVariable<T>>* GetVarsVec()
	{
		if constexpr (std::is_same_v<T, int>)
			return &varsi32;
		else if constexpr (std::is_same_v<T, float>)
			return &varsflt;
		else
		{
			LOGERROR("[FSM] Variables for given type don't exist in this fsm");
			return nullptr; 
		}
	}
};

}