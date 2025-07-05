#pragma once
namespace Toad
{
	class Scene;
	using json = nlohmann::ordered_json;
	

    // #TODO: Visualize & Fix
	class SceneHistory
	{
	public:
		SceneHistory(Scene* scene, const std::filesystem::path& asset_folder);
		SceneHistory();

	public:
		void Undo();
		void Redo();

		// returns whether a new scene state has been saved
		bool SaveState();
	
		Scene* scene;
		std::filesystem::path asset_folder;
		int history_length = 20;
	private:
		std::deque<json> m_undoableStates{};
		std::deque<json> m_redoableStates{};
	};
	
}
