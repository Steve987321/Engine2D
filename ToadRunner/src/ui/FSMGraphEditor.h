#pragma once 

#include "engine/utils/FSM.h"

struct ImVec2;

namespace Toad
{

class GameAssetsBrowser;

struct FSMGraphEditorNodeInfo
{
	ImVec2 pos;
	static inline ImVec2 node_prev_offset_pos{};
	static inline ImVec2 node_next_offset_pos{};
};

// Edit load and save FSMs.
class FSMGraphEditor
{
public:
	void Show(bool* show, const GameAssetsBrowser& asset_browser);
	bool LoadFromFile(const std::filesystem::path& path, const GameAssetsBrowser& asset_browser);
	bool SaveToFile(const std::filesystem::path& path);

	FSM* fsm = nullptr;
private:
	std::unordered_map<std::string, FSMGraphEditorNodeInfo> _statePos{};

};

}