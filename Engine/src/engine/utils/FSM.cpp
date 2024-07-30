#include "pch.h"
#include "FSM.h"

#include "engine/Engine.h"
#include "Helpers.h"

namespace Toad
{
	FSM::FSM(std::string_view name)
		: name(name)
	{
	}

	FSM::FSM()
		:name("fsm")
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
				if (m_currentState->on_exit)
					m_currentState->on_exit();
				t.Invoke();
				m_currentState = &m_states[t.m_nextStateIndex];
				if (m_currentState->on_enter)
					m_currentState->on_enter();
			}
		}
	}

	State* FSM::GetCurrentState() const
	{
		return m_currentState;
	}

	std::vector<State>& FSM::GetStates()
	{
		return m_states;
	}

	State* FSM::GetStateByName(std::string_view name)
	{
		for (State& state : m_states)
		{
			if (state.name == name)
				return &state;
		}
		return nullptr;
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
						state.name = state_name + '_' + std::to_string(i + 1);

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

	void FSM::AddVariable(std::string name, int var)
	{
		const std::string original_name = name;
		bool found = true;
		const auto fix_duplicate_name = [&]
			{
				found = false;
				for (int i = 0; i < varsi32.size(); i++)
				{
					if (name == varsi32[i].name)
					{
						found = true;

						// fix it and break and check again
						while (name == varsi32[i].name)
							name = original_name + '_' + std::to_string(i + 1);

						break;
					}

				}
			};

		while (found)
		{
			fix_duplicate_name();
		}
		varsi32.emplace_back(name, var);
	}

	void FSM::AddVariable(std::string name, float var)
	{
		const std::string original_name = name;
		bool found = true;
		const auto fix_duplicate_name = [&]
			{
				found = false;
				for (int i = 0; i < varsflt.size(); i++)
				{
					if (name == varsflt[i].name)
					{
						found = true;

						// fix it and break and check again
						while (name == varsflt[i].name)
							name = original_name + '_' + std::to_string(i + 1);

						break;
					}
				}
			};

		while (found)
		{
			fix_duplicate_name();
		}

		varsflt.emplace_back(name, var);
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
				transition_data["prev_state"] = m_states[transition.m_prevStateIndex].name;
				transition_data["next_state"] = m_states[transition.m_nextStateIndex].name;

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

		data["name"] = name;
		data["vars"] = vars_data;
		data["states"] = states_data;

		return data;
	}

	FSM FSM::Deserialize(const json& data)
	{
		FSM res("empty_fsm");

		json vars_data;
		json states_data;

		if (!get_json_element(res.name, data, "name"))
			res.name = "INVALID_NAME";

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
			int prev_state = 0;
			int next_state = 0;
			for (int i = 0; i < res.m_states.size(); i++)
			{
				State& state = res.m_states[i];
				if (state.name == prev_state_name)
					prev_state = i;
				else if (state.name == next_state_name)
					next_state = i;
			}

			if (!prev_state || !next_state)
				LOGERRORF("[FSM] Transition must have two states");
			else
			{
				Transition t(res, prev_state, next_state);

				for (const auto& item : i32_cond_data)
					t.AddConditionI32(TransitionCondition::Deserialize(item, res));
				for (const auto& item : flt_cond_data)
					t.AddConditionFlt(TransitionCondition::Deserialize(item, res));

				res.GetStates()[prev_state].transitions.emplace_back(t);
			}
		}

		return res;
	}

	FSM FSM::Deserialize(const std::filesystem::path& file)
	{
		std::ifstream f(file);
		if (!f)
			return FSM{ "empty_fsm" };

		json data;
		try 
		{
			data = json::parse(f);
		}
		catch (json::parse_error& e)
		{
			LOGERRORF("[FSM] Failed to parse fsm json data from file {}, {}", file.string(), e.what());
			return FSM{ "empty_fsm" };
		}

		return Deserialize(data);
	}

	Transition& Transition::operator=(const Transition& other)
	{
		if (this != &other)
		{
			m_nextStateIndex = other.m_nextStateIndex;
			m_prevStateIndex = other.m_prevStateIndex;
			conditions_i32 = other.conditions_i32;
			conditions_flt = other.conditions_flt;
			m_fsm = other.m_fsm;
		}
		return *this;
	}

	void Transition::Invoke()
	{
	}

	bool Transition::IsTransitionAllowed()
	{
		for (const TransitionCondition& conditioni32 : conditions_i32)
		{
			if (!conditioni32.IsConditionMet())
				return false;
		}
		for (const TransitionCondition& conditionflt : conditions_flt)
		{
			if (!conditionflt.IsConditionMet())
				return false;
		}

		return true;
	}

	void Transition::AddConditionI32(const TransitionCondition& condition_i32)
	{
		conditions_i32.emplace_back(condition_i32);
	}

	void Transition::AddConditionFlt(const TransitionCondition& condition_flt)
	{
		conditions_flt.emplace_back(condition_flt);
	}

	State* Transition::GetPreviousState()
	{
		std::vector<State>& states = m_fsm.GetStates();
		if (states.size() < m_prevStateIndex)
		{
			LOGERRORF("[FSMState] state index bigger than amount of states in fsm");
			return nullptr;
		}
		return &m_fsm.GetStates()[m_prevStateIndex];
	}

	State* Transition::GetNextState()
	{
		std::vector<State>& states = m_fsm.GetStates();
		if (states.size() < m_nextStateIndex)
		{
			LOGERRORF("[FSMState] state index bigger than amount of states in fsm");
			return nullptr;
		}

		return &m_fsm.GetStates()[m_nextStateIndex];
	}

	State::State(std::string_view name)
		: name(name)
	{
	}

	json TransitionCondition::Serialize() const
	{
		json data;
		data["a"] = a;
		data["b"] = b;
		data["compare"] = comparison_type;
		data["type"] = m_varType;
		return data;
	}

	TransitionCondition& TransitionCondition::operator=(const TransitionCondition& other)
	{
		if (this != &other)
		{
			a = other.a;
			b = other.b;
			comparison_type = other.comparison_type;
			m_varType = other.m_varType;
			m_fsm = other.m_fsm;
		}
		return *this;
	}

	TransitionCondition TransitionCondition::Deserialize(const json& data, FSM& fsm)
	{
		int a_index;
		int b_index;
		get_json_element(a_index, data, "a");
		get_json_element(b_index, data, "b");

		CompareType compare_type = CompareType::EQUAL;
		get_json_element(compare_type, data, "compare");

		FSMVariableType type;
		get_json_element(type, data, "type");

		return {&fsm, a_index, b_index, type, compare_type};
	}

	bool TransitionCondition::IsConditionMet() const
	{
		switch (m_varType)
		{
		case FSMVariableType::FLOAT:
			return CompareAB<float>(m_fsm->varsflt[a], m_fsm->varsflt[b]);
			break;
		case FSMVariableType::INT32:
			return CompareAB<int>(m_fsm->varsi32[a], m_fsm->varsi32[b]);
			break;
		default:
			return false;
			break;
		}
	}

	void to_string(CompareType type, char dest[3])
	{
		switch (type)
		{
		case CompareType::NOTEQUAL:
			strncpy(dest, "!=", 3);
			break;
		case CompareType::EQUAL:
			strncpy(dest, "==", 3);
			break;
		case CompareType::EQUALGREATERTHAN:
			strncpy(dest, ">=", 3);
			break;
		case CompareType::GREATERTHAN:
			strncpy(dest, ">", 3);
			break;
		case CompareType::EQUALLESSTHAN:
			strncpy(dest, "<=", 3);
			break;
		case CompareType::LESSTHAN:
			strncpy(dest, "<", 3);
			break;
		default:
			break;
		}
	}

	std::string to_string(CompareType type)
	{
		switch (type)
		{
		case CompareType::NOTEQUAL:
			return "!=";
		case CompareType::EQUAL:
			return "==";
		case CompareType::EQUALGREATERTHAN:
			return ">=";
		case CompareType::GREATERTHAN:
			return ">";
		case CompareType::EQUALLESSTHAN:
			return "<=";
		case CompareType::LESSTHAN:
			return "<";
		default:
			return "??";
		}
	}

}