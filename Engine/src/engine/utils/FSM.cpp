#include "pch.h"
#include "FSM.h"

#include "engine/Engine.h"
#include "Helpers.h"

namespace Toad
{
	FSM::FSM(std::string_view name)
		: m_name(name)
	{
	}

	FSM::FSM()
		:m_name("fsm")
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

	State* FSM::GetCurrentState() const
	{
		return m_currentState;
	}

	const std::string& FSM::GetName() const
	{
		return m_name;
	}

	std::vector<Toad::State>& FSM::GetStates()
	{
		return m_states;
	}

	void FSM::AddState(State state)
	{
		const std::string state_name = state.name;
		bool found = true;

		const auto fix_duplicate_name = [&]
		{
			found = false;
			for (int i = 0; i < m_states.size(); i++)
			{
				if (state.name == m_states[i].name)
				{
					found = true;

					// fix it and break and check again
					while (state.name == m_states[i].name)
						state.name = state_name + '_' + std::to_string(i++);

					break;
				}

			}
		};

		while (found)
		{
			fix_duplicate_name();
		}
		
		m_states.emplace_back(state);
	}

	void FSM::AddVariable(std::string_view name, int var)
	{
		varsi32.emplace_back(name.data(), var);
	}

	void FSM::AddVariable(std::string_view name, float var)
	{
		varsflt.emplace_back(name.data(), var);
	}

	json FSM::Serialize() const
	{
		json data;
		json states_data;
		json vars_data;

		json vars_flt_data;
		json vars_i32_data;

		for (const FSMVariable<float>& flt : varsflt)
			vars_flt_data[flt.name] = flt.data;
		for (const FSMVariable<int>& i32 : varsi32)
			vars_i32_data[i32.name] = i32.data;

		vars_data["flt"] = vars_flt_data;
		vars_data["i32"] = vars_i32_data;

		for (const State& state : m_states)
		{
			json state_data;
			json transitions_data;

			for (const Transition& transition : state.transitions)
			{
				json transition_data;
				transition_data["prev_state"] = transition.m_prevState.name;
				transition_data["next_state"] = transition.m_nextState.name;

				json flt_cond_data;
				json i32_cond_data;

				for (const auto& flt_cond : transition.conditions_flt)
					flt_cond_data.emplace_back(flt_cond.Serialize());
				for (const auto& i32_cond : transition.conditions_i32)
					i32_cond_data.emplace_back(i32_cond.Serialize());

				transition_data["flt_cond"] = flt_cond_data;
				transition_data["i32_cond"] = i32_cond_data;

				transitions_data.emplace_back(transition_data);
			}

			states_data[state.name] = transitions_data;
		}

		data["name"] = m_name;
		data["vars"] = vars_data;
		data["states"] = states_data;

		return data;
	}

	FSM FSM::Deserialize(const json& data)
	{
		FSM res("empty_fsm");

		json vars_data;
		json states_data;

		if (!get_json_element(res.m_name, data, "name"))
			res.m_name = "INVALID_NAME";

		get_json_element(vars_data, data, "vars");
		get_json_element(states_data, data, "states");

		// vars 
		json vars_flt_data;
		json vars_i32_data;
		get_json_element(vars_flt_data, vars_data, "flt");
		get_json_element(vars_i32_data, vars_data, "i32");

		for (auto& item : vars_flt_data.items())
			res.varsflt.emplace_back(item.key(), item.value());

		for (auto& item : vars_i32_data.items())
			res.varsi32.emplace_back(item.key(), item.value());

		json transition_data;
		// states 
		for (auto& item : states_data.items())
		{
			State state(item.key());

			for (auto& item2 : item.value().items())
			{
				// save transitions data for later, 
				// we need state data before we can create a transition 
				transition_data.emplace_back(item2.value());
			}
			res.AddState(state);
		}

		// transitions
		for (auto& transition : transition_data.items())
		{
			std::string prev_state_name;
			std::string next_state_name;

			json flt_cond_data;
			json i32_cond_data;
			get_json_element(prev_state_name, transition, "prev_state");
			get_json_element(next_state_name, transition, "next_state");
			get_json_element(flt_cond_data, transition, "flt_cond");
			get_json_element(i32_cond_data, transition, "i32_cond");

			// get states from names
			State* prev_state = nullptr;
			State* next_state = nullptr;
			for (State& state : res.m_states)
			{
				if (state.name == prev_state_name)
					prev_state = &state;
				else if (state.name == next_state_name)
					next_state = &state;
			}

			if (!prev_state || !next_state)
				LOGERRORF("[FSM] Transition must have two states");
			else
			{
				Transition t(*prev_state, *next_state);

				for (const auto& item : i32_cond_data)
					t.AddCondition(TransitionCondition<int>::Deserialize(item, res.varsi32));
				for (const auto& item : flt_cond_data)
					t.AddCondition(TransitionCondition<float>::Deserialize(item, res.varsflt));

				prev_state->transitions.emplace_back(t);
			}
		}

		return res;
	}

	Toad::Transition& Transition::operator=(const Transition& other)
	{
		if (this != &other)
		{
			m_prevState = other.m_prevState;
			m_nextState = other.m_nextState;
			conditions_i32 = other.conditions_i32;
			conditions_flt = other.conditions_flt;
		}
		return *this;
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

		return true;
	}

	void Transition::AddCondition(const TransitionCondition<int>& condition_i32)
	{
		conditions_i32.emplace_back(condition_i32);
	}

	void Transition::AddCondition(const TransitionCondition<float>& condition_flt)
	{
		conditions_flt.emplace_back(condition_flt);
	}

	State::State(std::string_view name)
		: name(name)
	{
	}

}