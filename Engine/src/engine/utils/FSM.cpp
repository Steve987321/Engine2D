#include "pch.h"
#include "FSM.h"

namespace Toad
{

	FSM::FSM()
	{

	}

	FSM::~FSM()
	{

	}

	void FSM::Update()
	{
		if (!m_currentState)
			return;

		for (Transition& t : m_currentState->transitions)
		{
			if (t.IsTransitionAllowed())
			{
				m_currentState->OnExit();
				t.Invoke();
				m_currentState = &t.m_nextState;
				m_currentState->OnEnter();
			}
		}
	}

	void FSM::AddState(State state)
	{
		m_states.emplace_back(state);
	}

	void FSM::AddVariable(std::string_view name, int var)
	{
		varsi.emplace_back(name, var);
	}

	void FSM::AddVariable(std::string_view name, float var)
	{
		varsflt.emplace_back(name, var);
	}

	void FSM::AddVariable(std::string_view name, std::string var)
	{
		varsstr.emplace_back(name, std::move(var));
	}

	json FSM::Serialize() const
	{
		json data;

		return data;
	}

	void Transition::Invoke()
	{

	}

	bool Transition::IsTransitionAllowed()
	{
		for (const TransitionCondition<int>& condition : conditions_i32)
		{
			if (!condition.IsConditionMet())
				return false;
		}
		for (const TransitionCondition<float>& condition : conditions_flt)
		{
			if (!condition.IsConditionMet())
				return false;
		}
		for (const TransitionCondition<std::string>& condition : conditions_str)
		{
			if (!condition.IsConditionMet())
				return false;
		}

		return true;
	}

	State::State(std::string_view name)
		: name(name)
	{
	}

}