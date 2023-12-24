#include "pch.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "Engine/Engine.h"

#include "GameAssetsBrowser.h"

#include "project/Misc.h"
#include "project/ToadProject.h"
#include "project/NewScriptClass.h"

#include "xml_parser/pugixml.hpp"

namespace Toad
{

bool ignore_rename_warning = false;

int rename_input_callback(ImGuiInputTextCallback* data)
{
	ignore_rename_warning = false;
	return 0;
}

GameAssetsBrowser::GameAssetsBrowser(std::string_view asset_path)
{
	SetAssetPath(asset_path);
}

GameAssetsBrowser::~GameAssetsBrowser()
{
}

void list_dir_contents(const fs::path& path)
{
	for (const auto& entry : fs::directory_iterator(path))
	{
		if (entry.is_directory())
		{
			if (ImGui::TreeNode(entry.path().filename().string().c_str()))
			{
				list_dir_contents(entry);

				ImGui::TreePop();
			}
		}
		else
		{
			if (ImGui::Button(entry.path().filename().string().c_str()))
			{
				// isnpecdt
				// double click open 
			}
		}
	}
}

void GameAssetsBrowser::Show()
{
	if (!m_assets_path.empty())
		list_dir_contents(m_assets_path);

	ImGui::Begin("second");

	if (m_assets_path.empty())
	{
		ImGui::Text("nothing to show");
		ImGui::End();
		return;
	}

	static bool is_dragging_file = false;

	if (is_dragging_file)
	{

	}
	else
	{
		if (m_current_path != m_assets_path && ImGui::ArrowButton("##back", ImGuiDir_Left))
		{
			m_current_path = m_current_path.parent_path();
		}
	}

	if (ImGui::Button("Open .sln"))
	{
		auto editors = misc::FindEditors();
		if (editors.empty())
		{
			LOGERROR("No visual studio found to open .sln");
		}
		else
		{
			for (const auto& editor : editors)
			{
				if (editor.name.find("Visual Studio") != std::string::npos)
				{
					// get .sln file
					// TODO: temp
					auto temp = m_assets_path.parent_path().parent_path().parent_path();
					fs::path slnpath;
					for (const auto& entry : fs::recursive_directory_iterator(temp))
					{
						if (entry.is_regular_file())
						{
							if (entry.path().extension() == ".sln")
							{
								slnpath = entry;
								break;
							}
						}
					}

					if (slnpath.empty())
					{
						LOGERRORF("can't find .sln file in {}", temp);
						break;
					}

					if (!project::OpenSln(slnpath, editor))
					{
						LOGERRORF("Failed to open {} with {}", slnpath, editor.name);
					}

					misc::current_editor = editor;

					break;
				}
			}
		}
	}

	static fs::path selected;
	static bool renaming = false;
	static char renaming_buf[100];

	static bool focus_on_popup_once = false;
	const ImGuiID create_cpp_script_popup = ImHashStr("create C++ script");

	// pop ups
	if (ImGui::BeginPopupModal("replace file warning"))
	{
		if (!focus_on_popup_once)
			ImGui::SetKeyboardFocusHere(-1);

		// re-rename
		ImGui::Text("This file already exists, do you wish to override this file?");
		ImGui::Separator();
		if (ImGui::Button("Yes"))
		{
			fs::rename(selected, selected.parent_path() / renaming_buf);
			renaming = false;
			ignore_rename_warning = false;
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Button("No"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGui::PushOverrideID(create_cpp_script_popup);
	if (ImGui::BeginPopupModal("create C++ script"))
	{
		static char script_name[50];

		if (ImGui::Button("Close"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::InputText("Name", script_name, sizeof(script_name));

		if (ImGui::Button("Create"))
		{
			if (CreateCPPScript(script_name))
			{
				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::EndPopup();
	}
	ImGui::PopID();

	focus_on_popup_once = ImGui::IsPopupOpen("replace file warning");

	if (ImGui::BeginPopup("creation menu"))
	{
		ImGui::SeparatorText("Add");

		if (ImGui::MenuItem("Directory"))
		{
			std::string dir_name = "new_directory";
			while (exists(m_current_path / dir_name))
			{
				dir_name += "_1";
			}
			create_directory(m_current_path / dir_name);

			selected = m_current_path / dir_name;

			strcpy_s(renaming_buf, selected.filename().string().c_str());
			ignore_rename_warning = true;
			renaming = true;

			ImGui::CloseCurrentPopup();
		}

		if (ImGui::MenuItem("Scene"))
		{
			std::string scene_name = "Scene";
			std::string file_ext = ".TSCENE";
			while (exists(m_current_path / (scene_name + file_ext)))
			{
				scene_name += "_1";
			}

			scene_name += file_ext;

			std::ofstream f(m_current_path / scene_name);
			nlohmann::json da;
			f << da;
			f.close();

			selected = m_current_path / scene_name;
			strcpy_s(renaming_buf, selected.filename().string().c_str());
			renaming = true;
			ignore_rename_warning = true;

			ImGui::CloseCurrentPopup();
		}
		if (ImGui::MenuItem("C++ Script"))
		{
			if (!ImGui::IsPopupOpen("create C++ script"))
			{
				ImGui::PushOverrideID(create_cpp_script_popup);
				ImGui::OpenPopup("create C++ script");
				ImGui::PopID();
			}
		}

		ImGui::EndPopup();
	}

	else if (ImGui::BeginPopup("modify menu"))
	{
		ImGui::SeparatorText(selected.filename().string().c_str());

		// TODO: Copy Cut Paste & Shortcuts
		if (ImGui::MenuItem("Copy", "CTRL+C"))
		{

			ImGui::CloseCurrentPopup();
		}
		if (ImGui::MenuItem("Cut", "CTRL+X"))
		{

			ImGui::CloseCurrentPopup();
		}

		// Paste

		if (ImGui::MenuItem("Delete"))
		{
			if (selected.has_extension() && selected.extension() == ".h" || selected.extension() == ".cpp")
			{
				do
				{
					if (!VerifyPaths())
					{
						LOGERROR("Failed to verify paths");
						break;
					}
					if (!RemoveFromScriptRegistry(selected))
					{
						LOGERRORF("Failed to remove {} from project", selected.filename().replace_extension("").string());
						break;
					}
					if (!ExcludeToProjectFile(selected))
					{
						LOGERRORF("Failed to remove {} from project", selected.filename());
						break;
					}
				} while (false);
			}
			fs::remove(selected);
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if (is_dragging_file && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		is_dragging_file = false;
	}

	int i = 0;
	for (const auto& entry : fs::directory_iterator(m_current_path))
	{
		i++;

		if (!selected.empty())
		{
			// rename
			if (!renaming && selected == entry.path() && ImGui::IsKeyPressed(ImGuiKey_F2))
			{
				strcpy_s(renaming_buf, entry.path().filename().string().c_str());
				renaming = true;
			}
		}

		if (entry.is_directory())
		{
			ImGui::PushID(i);
			if (ImGui::Selectable("D", selected == entry.path(), 0, { 50, 50 }))
			{
				selected = entry.path();
			}
			ImGui::PopID();

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
			{
				m_current_path = entry.path();
			}
		}
		else
		{
			ImGui::PushID(i);
			if (ImGui::Selectable("F", selected == entry.path(), 0, { 50, 50 }))
			{
				selected = entry.path();
			}
			ImGui::PopID();
		}


		ImGuiDragDropFlags src_flags = 0;
		src_flags |= ImGuiDragDropFlags_SourceNoDisableHover;
		src_flags |= ImGuiDragDropFlags_SourceNoHoldToOpenOthers;
		//src_flags |= ImGuiDragDropFlags_SourceNoPreviewTooltip;
		if (ImGui::BeginDragDropSource(src_flags))
		{
			auto buf = new std::string(entry.path().string());
			is_dragging_file = true;
			ImGui::SetDragDropPayload("move file", buf, entry.path().string().length());
			ImGui::BeginTooltip();
			ImGui::Text(fs::path(*buf).filename().string().c_str());
			ImGui::EndTooltip();
			ImGui::EndDragDropSource();
		}

		if (entry.is_directory() && ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("move file"))
			{
				fs::path src = *(std::string*)payload->Data;
				std::error_code e;
				fs::rename(src, entry.path() / src.filename(), e);
				LOGDEBUGF("error code message: {} {}", e.message(), e.value());
			}

			ImGui::EndDragDropTarget();
		}


		// options
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsItemHovered())
		{
			selected = entry.path();
			ImGui::OpenPopup("modify menu");
		}
		if (renaming && selected == entry.path())
		{
			if (!ImGui::IsPopupOpen("replace file warning"))
				ImGui::SetKeyboardFocusHere();

			if (ImGui::InputText("##", renaming_buf, 100, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackEdit, reinterpret_cast<ImGuiInputTextCallback>(rename_input_callback)))
			{
				if (!ignore_rename_warning && exists(entry.path().parent_path() / renaming_buf))
				{
					ImGui::OpenPopup("replace file warning");
				}
				else
				{
					fs::rename(entry.path(), entry.path().parent_path() / renaming_buf);
					renaming = false;
				}
			}
			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				strcpy_s(renaming_buf, entry.path().filename().string().c_str());
				renaming = false;
			}
		}
		else
		{
			ImGui::Text(entry.path().filename().string().c_str());
		}
	}

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
	{
		if (ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize()))
		{
			if (!ImGui::IsPopupOpen("modify menu"))
				ImGui::OpenPopup("creation menu");
		}

	}

	ImGui::End();
}

void GameAssetsBrowser::SetAssetPath(std::string_view path)
{
	m_assets_path = path;
	m_current_path = path;
}

const fs::path& GameAssetsBrowser::GetAssetPath()
{
	return m_assets_path;
}

bool GameAssetsBrowser::CreateCPPScript(std::string_view script_name)
{
	std::string cpp_file = (m_current_path / script_name).string() + ".cpp";
	std::string header_file = (m_current_path / script_name).string() + ".h";

	std::ofstream fcpp(cpp_file);
	if (!fcpp.is_open())
	{
		LOGERRORF("Failed to create file {}", cpp_file);
		return false;
	}

	std::ofstream fh(header_file);

	fcpp << format_str_ex(NewScriptCplusplus, '$', '@', script_name);
	fh << format_str_ex(NewScriptHeader, '$', '@', script_name);

	fcpp.close();
	fh.close();

	if (!VerifyPaths())
	{
		LOGERROR("Failed to verify paths");
		return false;
	}
	if (!AddToScriptRegistry(m_current_path / script_name))
	{
		LOGERRORF("Failed to add {} to script registry", script_name);
		return false;
	}
	if (!IncludeToProjectFile(cpp_file))
	{
		LOGERRORF("Failed to add {} to project file", cpp_file);
		return false;
	}
	if (!IncludeToProjectFile(header_file))
	{
		LOGERRORF("Failed to add {} to project file", header_file);
		return false;
	}

	return true;
}

bool GameAssetsBrowser::AddToScriptRegistry(const fs::path& script_path) const
{
	std::ifstream register_file(m_game_script_register_file);
	std::stringstream modified_register_file_content;
	modified_register_file_content << register_file.rdbuf();
	std::string str = modified_register_file_content.str();
	std::string file_relative = fs::relative(script_path, m_game_vsproj_file.parent_path() / "src").string();
	std::string script_name = fs::path(file_relative).filename().replace_extension("").string();

	if (!file_relative.ends_with(".h"))
	{
		file_relative += ".h";
	}
#ifdef _WIN32
	// replace to forward slashes
	for (char& c : file_relative)
	{
		if (c == '\\')
		{
			c = '/';
		}
	}
#endif

	if (str.find(format_str("#include \"{}\"", file_relative)) == std::string::npos)
	{
		std::istringstream ss(str);
		std::string line;
		std::string last_include;
		while (std::getline(ss, line))
		{
			if (size_t pos = line.find("#include"); pos != std::string::npos)
			{
				last_include = line;
			}
		}
		if (last_include.empty())
		{
			LOGERRORF("Invalid file: {}", m_game_script_register_file);
			return false;
		}

		for (size_t i = str.find(last_include); i < str.length(); i++)
		{
			if (str[i] == '\n')
			{
				str.insert(i, format_str("\n#include \"{}\"", file_relative));
				break;
			}
		}
	}
	
	if (str.find(format_str("SCRIPT_REGISTER({})", script_name)) != std::string::npos)
	{
		LOGDEBUGF("already registered {}", script_name);
	}
	else
	{
		if (size_t pos = str.find("register_scripts()");
			pos != std::string::npos)
		{
			size_t last_statement_pos = std::string::npos;
			bool valid = false;
			for (size_t i = pos; i < str.length(); i++)
			{
				if (str[i] == ';')
				{
					last_statement_pos = i + 1;
				}
				else if (str[i] == '}')
				{
					if (last_statement_pos != std::string::npos)
					{
						str.insert(last_statement_pos, format_str("\n\tSCRIPT_REGISTER({});", script_name));
						valid = true;
						break;
					}
				}
			}
			if (!valid)
			{
				LOGERRORF("register_scripts() is not valid in {}", m_game_script_register_file);
				register_file.close();
				return false;
			}
		}
		else
		{
			LOGERRORF("Can't find register_scripts() in {}", m_game_script_register_file);
			register_file.close();
			return false;
		}
	}

	register_file.close();

	std::ofstream new_register_file(m_game_script_register_file);
	new_register_file << str;
	new_register_file.close();
	return true;
}

bool GameAssetsBrowser::RemoveFromScriptRegistry(const fs::path& script_path) const
{
	std::ifstream register_file(m_game_script_register_file);
	std::ofstream modified_register_file(m_game_script_register_file.parent_path() / "temp.cpp");

	std::string script_name = script_path.filename().replace_extension("").string();
	std::string script_relative_path = fs::relative(script_path, m_game_vsproj_file.parent_path() / "src").replace_extension(".h").string();

#ifdef _WIN32
	for (char& c : script_relative_path)
	{
		if (c == '\\')
		{
			c = '/';
		}
	}
#endif

	std::string line;
	bool removed_line = false;
	while (std::getline(register_file, line))
	{
		if (line.find(format_str("SCRIPT_REGISTER({});", script_name)) != std::string::npos)
		{
			removed_line = true;
			continue;
		}
		if (line.find(format_str("#include \"{}\"", script_relative_path)) != std::string::npos)
		{
			removed_line = true;
			continue;
		}
		modified_register_file << line << '\n';
	}

	register_file.close();
	modified_register_file.close();

	if (!removed_line)
	{
		LOGDEBUG("didn't modify ScriptRegister");
		fs::remove(m_game_script_register_file.parent_path() / "temp.cpp");
	}
	else
	{
		fs::rename(m_game_script_register_file.parent_path() / "temp.cpp", m_game_script_register_file);
	}

	return true;
}

bool GameAssetsBrowser::IncludeToProjectFile(const fs::path& file_path_full)
{
	fs::path file_relative = fs::relative(file_path_full, m_game_vsproj_file.parent_path());
	bool is_header = file_relative.extension() == ".h";
	std::string xpath = is_header ? "//ItemGroup[ClInclude]" : "//ItemGroup[ClCompile]";
	std::string child_name = is_header ? "ClInclude" : "ClCompile";

	pugi::xml_document doc;

	pugi::xml_parse_result result = doc.load_file(m_game_vsproj_file.string().c_str());

	if (!result)
	{
		LOGERRORF("xml parse description: {} at offset {}", result.description(), result.offset);
		return false;
	}

	pugi::xml_node item_groupHeaders = doc.child("Project").select_node(xpath.c_str()).node();

	if (!item_groupHeaders)
	{
		LOGERROR("Failed to get ItemGroup");
		return false;
	}

	pugi::xml_node headerfile = item_groupHeaders.append_child(child_name.c_str());
	headerfile.append_attribute("Include").set_value(std::string(file_relative.string()).c_str());

	if (!doc.save_file(m_game_vsproj_file.string().c_str()))
	{
		LOGERRORF("Failed to save xml document: {}", m_game_vsproj_file.string().c_str());
		return false;
	}

	return true;
}

bool GameAssetsBrowser::ExcludeToProjectFile(const fs::path& file_path_full)
{
	fs::path file_relative = fs::relative(file_path_full, m_game_vsproj_file.parent_path());
	bool is_header = file_relative.extension() == ".h";

	std::string xpath = is_header ? "//ItemGroup[ClInclude]" : "//ItemGroup[ClCompile]";
	std::string child_name = is_header ? "ClInclude" : "ClCompile";

	pugi::xml_document doc;

	pugi::xml_parse_result result = doc.load_file(m_game_vsproj_file.string().c_str());

	if (!result)
	{
		LOGERRORF("xml parse description: {} at offset {}", result.description(), result.offset);
		return false;
	}

	pugi::xml_node item_groupHeaders = doc.child("Project").select_node(xpath.c_str()).node();

	if (!item_groupHeaders)
	{
		LOGERROR("Failed to get ItemGroup");
		return false;
	}

	for (pugi::xml_node ClInclude = item_groupHeaders.child(child_name.c_str()); ClInclude; ClInclude = ClInclude.next_sibling(child_name.c_str()))
	{
		if (ClInclude.attribute("Include").as_string() == file_relative.string())
		{
			item_groupHeaders.remove_child(ClInclude);
			break;
		}
	}

	if (!doc.save_file(m_game_vsproj_file.string().c_str()))
	{
		LOGERRORF("Failed to save xml document: {}", m_game_vsproj_file.string().c_str());
		return false;
	}

	return true;
}

bool GameAssetsBrowser::VerifyPaths()
{
	if (m_game_vsproj_file.empty())
	{
		for (const auto& entry : fs::recursive_directory_iterator(project::current_project.project_path))
		{
			if (entry.path().has_extension() && entry.path().extension() == ".vcxproj")
			{
				if (entry.path().filename().string().find("_Game") != std::string::npos)
				{
					m_game_vsproj_file = entry.path();
					break;
				}
			}
		}
	}

	if (m_game_vsproj_file.empty())
	{
		LOGERRORF("Failed to find game vcxproj file in {}", project::current_project.project_path);
		return false;
	}

	if (m_game_script_register_file.empty())
	{
		m_game_script_register_file = m_game_vsproj_file.parent_path() / "src" / "game_core" / "ScriptRegister.cpp";
		if (!fs::exists(m_game_script_register_file))
		{
			LOGERRORF("can't find script register {}", m_game_script_register_file);
			return false;
		}
	}

	return true;
}

}
