#pragma once 

#include "engine/utils/FSM.h"

struct ImVec2;

namespace ui
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
	FSMGraphEditor();

	void Show(bool* show, const GameAssetsBrowser& asset_browser);
	bool LoadFromFile(const std::filesystem::path& path, const GameAssetsBrowser& asset_browser);
	bool SaveToFile(const std::filesystem::path& path);

	Toad::FSM* fsm = nullptr;
private:
	void ShowFSMProps();
	std::function<void()> m_inspectorUI;

	bool m_simulateFSM = false;
	Toad::json m_previousFSMState{};

	std::filesystem::path m_savedPath{};
	char m_savedPathBuf[MAX_PATH]{};

	static constexpr ImU32 transition_line_col = IM_COL32_WHITE;
	static constexpr ImU32 transition_active_line_col = IM_COL32_WHITE;

	std::unordered_map<std::string, FSMGraphEditorNodeInfo> m_statePos{};

};

}