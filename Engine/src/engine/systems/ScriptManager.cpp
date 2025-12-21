#include "pch.h"
#include "ScriptManager.h"

#include "engine/default_scripts/Script.h"
#include "engine/systems/Scene.h"
#include "engine/systems/Input.h"
#include "engine/utils/DLib.h"
#include "engine/systems/Timer.h"

#include "engine/Settings.h" 
#include "engine/Logger.h"

#include "game_core/ScriptRegister.h"

namespace Toad
{
	// instances of all game scripts
	static TGAME_SCRIPTS game_scripts = {};
	static DllHandle current_dll;

	void ScriptManager::LoadScripts()
	{
		namespace fs = std::filesystem;
		using object_script = struct { std::string script_name; ReflectionCopy reflection; };
		std::unordered_map <std::string, std::vector<object_script>> objects_with_scripts{};

		for (auto& [_, script] : game_scripts)
		{
			if (script)
				free(script);

			script = nullptr;
		}

		for (auto& obj : Scene::current_scene.objects_all)
		{
			auto& scripts = obj->GetAttachedScripts();
			if (scripts.empty())
				continue;

			for (auto i = scripts.begin(); i != scripts.end();)
			{
				ReflectionCopy vars = i->second->GetReflection().Copy();
				objects_with_scripts[obj->name].emplace_back(object_script{ i->first, vars });

				obj->RemoveScript(i->first);
				i = scripts.begin();
			}
		}

		// clear Input callbacks 
		Input::Clear();

    #ifdef __APPLE__
        Input::AddKeyPressCallback(InputMac::MacKeyPressCallback);
        Input::AddKeyReleaseCallback(InputMac::MacKeyReleaseCallback);
    #endif 

		if (current_dll)
		{
			DLibFree(current_dll);
		}

		fs::path game_dll_path = game_bin_directory + game_bin_file;
		fs::path current_game_dll = game_bin_directory + LIB_FILE_PREFIX + "GameCurrent" + LIB_FILE_EXT;
		// fs::path current_game_dll = game_bin_directory + game_bin_file;

#ifdef TOAD_EDITOR
		if (fs::exists(current_game_dll))
		{
			if (fs::exists(game_dll_path))
			{
				if (!fs::remove(current_game_dll.c_str()))
				{
					LOGERRORF("[ScriptManager] Failed to remove file {}", current_game_dll);
				}
			}
		}
#endif

		if (!fs::exists(game_dll_path))
		{
			LOGWARNF("[ScriptManager] Couldn't find game dll file, {}", game_dll_path);
		}
		else 
		{
			std::string err;
			Timer t;
			int tries = 0;
			do
			{
				try
				{
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
					fs::rename(game_dll_path, current_game_dll);
					tries++;
				}
				catch (fs::filesystem_error& e)
				{
					err = e.what();
				}
			} while (t.Elapsed<std::chrono::milliseconds>() < 2000);

			if (!err.empty())
				LOGERRORF("[ScriptManager] Rename failed: {}", err);
		}

		auto dll = DLibOpen(current_game_dll.string());
		if (!dll)
		{
			LOGERRORF("[ScriptManager] Couldn't load game dll file, {} : {}", current_game_dll, DLGetError());
			return;
		}

		current_dll = dll;

		auto registerScripts = reinterpret_cast<register_scripts_t*>(DLibGetAddress(dll, "register_scripts"));
		auto getScripts = reinterpret_cast<get_registered_scripts_t*>(DLibGetAddress(dll, "get_registered_scripts"));

		registerScripts();

		Scripts scripts_data = getScripts();
		if (!scripts_data.scripts)
		{
			LOGERROR("[ScriptManager] scripts_data is nullptr");
			return;
		}

		for (size_t i = 0; i < scripts_data.len; i++)
		{
			const auto& [b, n] = scripts_data.scripts[i];

			Script* script = (Script*)(b);
			if (!script)
			{
				LOGERRORF("[ScriptManager] Script is nullptr: {}", i);
				continue;
			}
			LOGDEBUGF("[ScriptManager] Loading script {}. Size: {}", script->GetName(), n);
			void* p = malloc(n);
			memcpy(p, b, n);

			// this can be deleted after 
			assert(p && "Allocation failed for script, please rebuild script to be updated");
			game_scripts[script->GetName()] = (Script*)p;
			game_scripts[script->GetName()]->ExposeVars();
		}
		for (TGAME_SCRIPTS::iterator it = game_scripts.begin(); it != game_scripts.end();)
		{
			if (!it->second)
			{
				LOGWARNF("[ScriptManager] Script {} is now null and is getting removed", it->first.c_str());
				it = game_scripts.erase(it);
			}
			else
			{
				++it;
			}
		}

		// update scripts on object if it has an old version

		for (auto& obj : Scene::current_scene.objects_all)
		{
			if (!objects_with_scripts.contains(obj->name))
				continue;

			auto& prev_obj_state = objects_with_scripts[obj->name];

			for (auto& [attached_script_name, old_reflection_vars] : prev_obj_state)
			{
				auto it = game_scripts.find(attached_script_name);
				if (it != game_scripts.end())
				{
					// update exposed vars if they exist 
					// LOGDEBUGF("[Engine] Alloc for script {} with size {}", it->first, sizeof(*it->second));
					// void* p = malloc(sizeof(*it->second));
					// memcpy(p, it->second, sizeof(*it->second));
					obj->AddScript(it->second->Clone());
					obj->GetScript(it->first)->ExposeVars();
					auto& new_reflection_vars = obj->GetScript(it->first)->GetReflection();

                    new_reflection_vars.Update(old_reflection_vars);

					LOGDEBUGF("[ScriptManager] Updated Script {} on Object {}", attached_script_name, obj->name);
				}
			}

			objects_with_scripts.erase(obj->name);
		}

#ifdef _DEBUG
		for (const auto& [obj_name, info] : objects_with_scripts)
		{
			for (const auto& script : info)
			{
				LOGWARNF("[ScriptManager] Script lost {} on object {}", script.script_name, obj_name);
			}
		}
#endif

		// get the important script functions
//
//		for (auto& [name, script] : game_scripts)
//		{
//            const std::function<void(Toad::Object* obj)> f = std::bind(&Script::OnUpdate, script, std::placeholders::_1);
//		}
	}

	void ScriptManager::SetBinPaths(std::string_view game_bin_file_name, std::string_view bin_path)
	{
		game_bin_directory = bin_path;
		game_bin_file = game_bin_file_name;

		if (!game_bin_directory.ends_with(PATH_SEPARATOR))
		{
			game_bin_directory += PATH_SEPARATOR;
		}
	}

	void ScriptManager::CallStartScripts()
	{

	}

	void ScriptManager::CallUpdateScripts()
	{

	}

	TGAME_SCRIPTS& ScriptManager::GetScripts()
	{
		return game_scripts;
	}

}
