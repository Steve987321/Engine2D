#include "pch.h"
#include "nlohmann/json.hpp"
#include "engine/Engine.h"
#include "SceneHistory.h"

namespace Toad
{
	SceneHistory::SceneHistory(Scene* scene, const std::filesystem::path& asset_folder)
		: scene(scene), asset_folder(asset_folder)
	{
	}

	SceneHistory::SceneHistory()
		: scene(nullptr), asset_folder()
	{
	}

	void SceneHistory::Undo()
	{
		assert(scene && "scene in SceneHistory is not valid");

		if (m_undoableStates.size() <= 1)
		{
			return;
		}

		m_redoableStates.push_front(m_undoableStates.front());
		m_undoableStates.pop_front();
		json last_state = m_undoableStates.front();

		scene->objects_all.clear();

		LoadSceneObjects(last_state, *scene, asset_folder);
		

		while (m_redoableStates.size() > history_length)
		{
			m_redoableStates.pop_back();
		}
	}

	void SceneHistory::Redo()
	{
		assert(scene && "scene in SceneHistory is not valid");

		if (m_redoableStates.empty())
		{
			return;
		}

		m_undoableStates.push_front(m_redoableStates.front());
		json last_state = m_redoableStates.front();
		m_redoableStates.pop_front();

		scene->objects_all.clear();

		LoadSceneObjects(last_state, *scene, asset_folder);


		while (m_undoableStates.size() > history_length)
		{
			m_undoableStates.pop_back();
		}
	}

	bool SceneHistory::SaveState()
	{
		assert(scene && "scene in SceneHistory is not valid");

		json data = scene->Serialize();

		if (!m_undoableStates.empty())
		{
			if (m_undoableStates.front() == data)
			{
				// LOGDEBUGF("[SceneHistory][Scene:{}] same data & is getting skipped", scene->name);
				return false;
			}
		}
		
		m_undoableStates.push_front(data);

		while (m_undoableStates.size() > history_length)
		{
			m_undoableStates.pop_back();
		}

		return true;
	}

}
