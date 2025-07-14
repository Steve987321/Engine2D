#include "pch.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "Viewport.h"
#include "GameAssetsBrowser.h"
#include "Inspector.h"

#include "imgui-SFML.h"
#include "engine/Engine.h"
#include "engine/PlaySession.h"

#include "UI.h"

using namespace Toad;
namespace fs = std::filesystem;

namespace ui
{
	void ShowViewport(GameAssetsBrowser& asset_browser)
	{
		bool viewport_opened = ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_HorizontalScrollbar);
		{
			auto& texture = Toad::GetEditorCameraTexture();
			const ImVec2 content_size = ImGui::GetContentRegionAvail();

			Camera& editor_cam = Toad::GetEditorCamera();

			static Vec2f initial_editor_cam_size = editor_cam.GetSize();

			constexpr float ar = 16.f / 9.f;
			float image_width = content_size.x;
			float image_height = content_size.x / ar;

			float fscale_x = image_width / editor_cam.GetSize().x;
			float fscale_y = image_height / editor_cam.GetSize().y;

			if (image_height > content_size.y)
			{
				image_height = content_size.y;
				image_width = content_size.y * ar;
			}

			float pady = 25; // #TODO: find the actual imgui style property 
			ImGui::SetCursorPos({
				(content_size.x - image_width) * 0.5f,
				(content_size.y - image_height + pady) * 0.5f
				});

			//editor_cam.SetSize({content_size.x, content_size.y});

			const auto pos = ImGui::GetCursorScreenPos();

			ImVec2 image_cursor_pos = ImGui::GetCursorPos();
			ImGui::Image(texture, { image_width, image_height }, sf::Color::White);

			if (ImGui::IsWindowHovered())
			{
				SetInteractingCamera(&editor_cam);
				SetInteractingTexture(&texture);

				viewport_size = { (int)image_width, (int)image_height };
				float f_x = editor_cam.GetSize().x / image_width;
				float f_y = editor_cam.GetSize().y / image_height;
				
				Mouse::SetRelativeMousePosition({
					(int)((ImGui::GetMousePos().x - pos.x) * f_x),
					(int)((ImGui::GetMousePos().y - pos.y) * f_y)});
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dnd tilesheettile"))
				{
					SheetTileData data = *(SheetTileData*)payload->Data;

					sf::Texture* stex = ResourceManager::GetTextures().Get(data.path);
					if (!stex)
					{
						LOGERRORF("[UI:Viewport Drag Drop tilesheet] texture can't be loaded from resourcemanager: {}", data.path);
					}
					else
					{
						Sprite* added_obj = Scene::current_scene.AddToScene(Sprite("Sprite"), Toad::begin_play).get();

						added_obj->SetTexture(data.path, stex);
						added_obj->GetSprite().setTextureRect(data.tex_rect);
						added_obj->GetSprite().setScale(data.tex_size);
						ImVec2 curr_pos = { ImGui::GetMousePos().x - pos.x, ImGui::GetMousePos().y - pos.y };
						float fx = editor_cam.GetSize().x / image_width;
						float fy = editor_cam.GetSize().y / image_height;
						Vec2f pos = { curr_pos.x * fx, curr_pos.y * fy };

						pos = texture.mapPixelToCoords({ (int)pos.x, (int)pos.y }, editor_cam.GetView());

						added_obj->SetPosition(pos);
					}
				}

				ImGui::EndDragDropTarget();
			}
			ImGui::GetWindowDrawList()->AddText({ pos.x + content_size.x - 70, pos.y + 10 }, IM_COL32_WHITE, format_str("CW:{}\nCH:{}", content_size.x, content_size.y).c_str());
			ImGui::GetWindowDrawList()->AddText({ pos.x + content_size.x - 70, pos.y + 40 }, IM_COL32_WHITE, format_str("IW:{}\nIH:{}", image_width, image_height).c_str());

			std::vector<ImVec2> positions;
			static bool always_show_object_names = true;

			const auto obj_screen_pos_info = [&](Object* obj, bool add_to_list = true)
				{
					if (obj)
					{
						Vec2i obj_pos_px = texture.mapCoordsToPixel(obj->GetPosition(), editor_cam.GetView());
						Vec2f obj_pos_px_flt = { (float)obj_pos_px.x, (float)obj_pos_px.y };

						float scale_x = image_width / initial_editor_cam_size.x;
						float scale_y = image_height / initial_editor_cam_size.y;

						obj_pos_px_flt.x *= scale_x;
						obj_pos_px_flt.y *= scale_y;
						obj_pos_px_flt.x += pos.x;
						obj_pos_px_flt.y += pos.y;

						if (add_to_list)
						{
							positions.emplace_back(obj_pos_px_flt.x, obj_pos_px_flt.y);
							ImGui::GetWindowDrawList()->AddText(ImVec2{ obj_pos_px_flt.x, obj_pos_px_flt.y }, IM_COL32(255, 255, 0, 160), obj->name.c_str());
						}
						else
						{
							ImGui::GetWindowDrawList()->AddText(ImVec2{ obj_pos_px_flt.x, obj_pos_px_flt.y }, IM_COL32(255, 255, 0, 100), obj->name.c_str());
						}
					}
				};

			if (always_show_object_names)
			{
				for (const auto& obj : Scene::current_scene.objects_all)
				{
					if (obj.get() == selected_obj || selected_objects.contains(obj->name))
					{
						obj_screen_pos_info(selected_obj);
					}
					else
					{
						obj_screen_pos_info(obj.get(), false);
					}
				}
			}
			if (selected_obj)
			{
				obj_screen_pos_info(selected_obj);
			}
			for (const auto& name : selected_objects)
			{
				Object* obj = Scene::current_scene.GetSceneObject(name).get();
				if (obj)
				{
					obj_screen_pos_info(obj);
				}
			}

			// #TODO abstract gizmo ass 
			static ImVec2 select_begin_relative = {};
			static ImVec2 select_begin_cursor = {};
			static bool is_moving_gizmo = false;
			static bool is_gizmo_pos = false;
			static bool is_gizmo_scale = false;
			static bool is_gizmo_rotation = false;
			static ImVec2 gizmo_begin_cursor = {};
			static Vec2f starting_scale = {};
			static float starting_rotation = 0;
			static Vec2f starting_position = {};

			ImVec2 gizmo_pos;
			static Vec2i selected_gizmo = { 0, 0 }; // #TODO enum 
			static ImU32 gizmo_col_xy = ImGui::ColorConvertFloat4ToU32({ 1, 0, 0, 0.5f });
			static ImU32 gizmo_col_x = ImGui::ColorConvertFloat4ToU32({ 0, 0, 1, 0 });
			static ImU32 gizmo_col_y = ImGui::ColorConvertFloat4ToU32({ 0, 1, 0, 0 });

			ImVec2 gizmo_xy_size = { 10, 10 };
			const float gizmo_line_size = 40;
			const float gizmo_line_width = 2.f;

			for (const ImVec2& position : positions)
			{
				gizmo_pos.x += position.x;
				gizmo_pos.y += position.y;
			}

			gizmo_pos.x /= positions.size();
			gizmo_pos.y /= positions.size();

			ImGui::GetWindowDrawList()->AddRectFilled(gizmo_pos - gizmo_xy_size, gizmo_pos + gizmo_xy_size, gizmo_col_xy);
			// y
			ImGui::GetWindowDrawList()->AddLine(gizmo_pos, { gizmo_pos.x, gizmo_pos.y + gizmo_line_size }, gizmo_col_y, gizmo_line_width);
			// x
			ImGui::GetWindowDrawList()->AddLine(gizmo_pos, { gizmo_pos.x + gizmo_line_size, gizmo_pos.y }, gizmo_col_x, gizmo_line_width);

			// draw grid 
			//for (int i = 0; i < content_size.x / grid_size.x; i++)
			{
				//ImGui::GetWindowDrawList()->AddLine( pos + ImVec2{i * grid_size.x, 0}, pos + ImVec2{i * grid_size.x, content_size.y}, IM_COL32_WHITE);
			}

			if (is_moving_gizmo)
			{
				ImVec2 d;
				if (is_gizmo_pos)
					d = ImGui::GetMouseDragDelta(0, 0.f);
				else
					d = ImGui::GetMousePos() - gizmo_begin_cursor;

				static bool temp_disable_snapping = false;
				if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && drag_snap)
				{
					temp_disable_snapping = true;
					drag_snap = false;
				}

				if (ImGui::IsKeyReleased(ImGuiKey_LeftShift) && temp_disable_snapping)
				{
					temp_disable_snapping = false;
					drag_snap = true;
				}

				if (ImGui::IsKeyPressed(ImGuiKey_X))
				{
					selected_gizmo.x = ~selected_gizmo.x;
					if (selected_gizmo.y)
					{
						selected_gizmo.y = ~selected_gizmo.y;
					}
					if (selected_gizmo.x)
					{
						gizmo_col_x = ImGui::ColorConvertFloat4ToU32({ 0, 0, 1, 0.8f });
						gizmo_col_y = ImGui::ColorConvertFloat4ToU32({ 0, 0, 1, 0 });
					}
					else
					{
						gizmo_col_x = ImGui::ColorConvertFloat4ToU32({ 0, 0, 1, 0 });
					}
				}
				else if (ImGui::IsKeyPressed(ImGuiKey_Y))
				{
					selected_gizmo.y = ~selected_gizmo.y;
					if (selected_gizmo.x)
					{
						selected_gizmo.x = ~selected_gizmo.x;
					}
					if (selected_gizmo.y)
					{
						gizmo_col_y = ImGui::ColorConvertFloat4ToU32({ 0, 1, 0, 0.8f });
						gizmo_col_x = ImGui::ColorConvertFloat4ToU32({ 0, 0, 1, 0 });

					}
					else
					{
						gizmo_col_y = ImGui::ColorConvertFloat4ToU32({ 0, 1, 0, 0 });
					}
				}

				if (d.x || d.y)
				{
					if (selected_gizmo.x)
						d.y = 0;
					else if (selected_gizmo.y)
						d.x = 0;

					float multiplierx = editor_cam.GetSize().x / image_width;
					float multipliery = editor_cam.GetSize().y / image_height;

					d.x *= multiplierx;
					d.y *= multipliery;

					// absolutely horrendous 

					if (selected_obj)
					{
						if (drag_snap)
						{
							Vec2f drag_pos = selected_obj->GetPosition() + Vec2f{ d.x, d.y };

							const Vec2i& mouse_pos = Mouse::GetRelativeMousePosition();
							Vec2f mouse_world_pos = Screen::ScreenToWorld(mouse_pos);

							// snap to the grid
							mouse_world_pos.x = std::round(mouse_world_pos.x / grid_size.x) * grid_size.x;
							mouse_world_pos.y = std::round(mouse_world_pos.y / grid_size.y) * grid_size.y;

							selected_obj->SetPosition(mouse_world_pos);

							//FloatRect selected_obj_bounds = selected_obj->GetBounds();

							//if (selected_obj_bounds.width != 0 && selected_obj_bounds.height != 0)
							//{
							//	for (const auto& obj : Scene::current_scene.objects_all)
							//	{
							//		if (selected_obj == obj.get())
							//			continue;

							//		FloatRect bounds = obj->GetBounds();
							//		if (bounds.width == 0 || bounds.height == 0)
							//			continue;
							//		
							//		bounds.Expand(5.f);
							//		if (bounds.Contains(mouse_world_pos))
							//		{
							//			// snap to nearest edge
							//			float right = bounds.left + bounds.width;
							//			float bottom = bounds.top + bounds.height;

							//			if (mouse_world_pos.x > right)
							//			{

							//			}
							//		}
							//	}
							//}
						}
						else
						{
							LOGDEBUGF("{} {}", d.x, d.y);
							if (is_gizmo_pos)
								selected_obj->SetPosition(selected_obj->GetPosition() + Vec2f{ d.x, d.y });
							else if (is_gizmo_scale)
							{
								selected_obj->SetScale(starting_scale + Vec2f{ d.x, d.y } / 4.f);
							}
							else if (is_gizmo_rotation)
							{
								float deg = RadToDeg(std::atan2f(d.y, d.x));
								selected_obj->SetRotation(starting_rotation + deg);
							}
						}
					}
					for (const std::string& name : selected_objects)
					{
						Object* obj = Scene::current_scene.GetSceneObject(name).get();
						if (obj)
							obj->SetPosition(obj->GetPosition() + Vec2f{ d.x, d.y });
					}

					ImGuiContext* g = ImGui::GetCurrentContext();
					*g->IO.MouseClickedPos = ImGui::GetMousePos();
				}
				if (is_gizmo_pos && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
				{
					selected_gizmo = { 0, 0 };
					gizmo_col_x = ImGui::ColorConvertFloat4ToU32({ 0, 0, 0, 0 });
					gizmo_col_y = ImGui::ColorConvertFloat4ToU32({ 0, 0, 0, 0 });
					is_moving_gizmo = false;
					is_gizmo_rotation = false;
					is_gizmo_scale = false;
					is_gizmo_pos = false;
				}
				else
				{
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						selected_gizmo = { 0, 0 };
						gizmo_col_x = ImGui::ColorConvertFloat4ToU32({ 0, 0, 0, 0 });
						gizmo_col_y = ImGui::ColorConvertFloat4ToU32({ 0, 0, 0, 0 });
						is_moving_gizmo = false;
						is_gizmo_rotation = false;
						is_gizmo_scale = false;
						is_gizmo_pos = false;
					}
					//else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
					//{
					//	if (!scene_history.scene)
					//	{
					//		if (is_gizmo_pos)
					//			selected_obj->SetPosition(starting_position);
					//		else if (is_gizmo_rotation)
					//			selected_obj->SetRotation(starting_rotation);
					//		else if (is_gizmo_scale)
					//			selected_obj->SetScale(starting_scale);
					//	}
					//	else
					//		scene_history.Undo();

					//	selected_gizmo = { 0, 0 };
					//	gizmo_col_x = ImGui::ColorConvertFloat4ToU32({ 0, 0, 0, 0 });
					//	gizmo_col_y = ImGui::ColorConvertFloat4ToU32({ 0, 0, 0, 0 });
					//	is_moving_gizmo = false;
					//	is_gizmo_rotation = false;
					//	is_gizmo_scale = false;
					//	is_gizmo_pos = false;
					//}
				}
			}
			else
			{
				// xy 
				if (ImRect((gizmo_pos - gizmo_xy_size), gizmo_pos + gizmo_xy_size).Contains(ImGui::GetMousePos()))
				{
					gizmo_col_xy = ImGui::ColorConvertFloat4ToU32({ 1, 0, 0, 1 });

					if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						starting_rotation = 0;
						starting_scale = Vec2f{};
                        if (selected_obj)
                            starting_position = selected_obj->GetPosition();

						is_moving_gizmo = true;
						is_gizmo_pos = true;
					}
					else
					{
						is_moving_gizmo = false;
						is_gizmo_pos = false;
					}
				}
				else
				{
					gizmo_col_xy = ImGui::ColorConvertFloat4ToU32({ 1, 0, 0, 0.5f });
				}

				//// scale
				//if (ImGui::IsKeyPressed(ImGuiKey_S))
				//{
				//	gizmo_begin_cursor = ImGui::GetMousePos();
				//	is_moving_gizmo = true;
				//	is_gizmo_scale = true;
				//	starting_scale = selected_obj->GetScale();
				//}

				//// rotation
				//if (ImGui::IsKeyPressed(ImGuiKey_R))
				//{
				//	gizmo_begin_cursor = ImGui::GetMousePos();
				//	is_moving_gizmo = true;
				//	is_gizmo_rotation = true;
				//	starting_rotation = selected_obj->GetRotation();
				//}
			}

			if (viewport_opened && ImGui::IsWindowFocused())
			{
				if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
				{
                    // #TODO: multiply by some zoom factor
                    Vec2f sens_factor = editor_cam.GetSize() / editor_cam.original_size;
                    
					ImGuiContext* g = ImGui::GetCurrentContext();
					ImVec2 d = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle, 0.f);
					g->IO.MouseClickedPos[ImGuiMouseButton_Middle] = ImGui::GetMousePos();
					editor_cam.SetPosition(editor_cam.GetPosition() - Vec2f(d.x * sens_factor.x, d.y * sens_factor.y));
				}

				float mwheel = ImGui::GetIO().MouseWheel;
				if (mwheel)
				{
					if (mwheel < 0)
					{
						editor_cam.Zoom(1.2f);
					}
					else
					{
						editor_cam.Zoom(0.8f);
					}
				}
			}

			if (ImGui::IsItemHovered())
			{
				if (!is_moving_gizmo)
				{
					if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
					{
						ImGui::GetWindowDrawList()->AddRectFilled(select_begin_cursor, select_begin_cursor + ImGui::GetMouseDragDelta(), IM_COL32(155, 155, 255, 50), 0);
						auto texture_size = texture.getSize();

						ImVec2 curr_pos = { ImGui::GetMousePos().x - pos.x, ImGui::GetMousePos().y - pos.y };

						//float fx = editor_cam.GetSize().x / image_width;
						//float fy = editor_cam.GetSize().y / image_height;
						float x1 = select_begin_relative.x + pos.x;
						float y1 = select_begin_relative.y + pos.y;
						float x2 = curr_pos.x + pos.x;
						float y2 = curr_pos.y + pos.y;

						//LOGDEBUGF("{} {}  {} {}", x1, y1, x2, y2);

						ImRect rect(std::min(x1, x2), std::min(y1, y2), std::max(x1, x2), std::max(y1, y2));

						for (const auto& obj : Scene::current_scene.objects_all)
						{
							Vec2i a = texture.mapCoordsToPixel(obj->GetPosition(), editor_cam.GetView());
							Vec2f a_flt = { (float)a.x, (float)a.y };

							float scale_x = image_width / initial_editor_cam_size.x;
							float scale_y = image_height / initial_editor_cam_size.y;

							a_flt.x *= scale_x;
							a_flt.y *= scale_y;
							a_flt.x += pos.x;
							a_flt.y += pos.y;

							if (rect.Contains({ a_flt.x, a_flt.y })) {
								if (selected_obj != obj.get())
									selected_objects.emplace(obj->name);
								else
									selected_obj = obj.get();
							}
							else
							{
								if (!ImGui::IsKeyDown(ImGuiKey_LeftShift))
								{
									if (selected_obj == obj.get())
										selected_obj = nullptr;
									else if (selected_objects.contains(obj->name))
										selected_objects.erase(obj->name);
								}
							}
						}
					}
					else if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
					{
						//LOGDEBUGF("{} {} {} {}", ImGui::GetMousePos().x, ImGui::GetMousePos().y, image_cursor_pos.x, image_cursor_pos.y);
						select_begin_cursor = ImGui::GetMousePos();
						select_begin_relative = { ImGui::GetMousePos().x - pos.x, ImGui::GetMousePos().y - pos.y };

						//if (!ImGui::IsAnyItemHovered())
						//{
						//	selected_obj = nullptr;
						//	selected_objects.clear();
						//}
					}

					if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
					{
						// only selected one object
						if (selected_objects.size() == 1 && !selected_obj)
						{
							selected_obj = Scene::current_scene.GetSceneObject(*selected_objects.begin()).get();
							selected_objects.erase(selected_objects.begin());
						}

						inspector_ui = std::bind(&ui::object_inspector, std::ref(selected_obj), std::ref(asset_browser));
					}
				}
			}

			ImGui::SetCursorPos({ ImGui::GetScrollX() + 20, 20 });
			if (ImGui::TreeNode("Viewport Options"))
			{
				static fs::path last_scene_path;

				if (!Toad::begin_play)
				{
					if (ImGui::Button("Play"))
					{
						if (reload_scene_on_stop)
							last_scene_path = Scene::current_scene.path;

						Toad::StartGameSession();
					}
				}
				else
				{
					if (ImGui::Button("Stop"))
					{
						Toad::StopGameSession();
                        
						if (reload_scene_on_stop)
                        {
                            selected_obj = nullptr; // after reload reset this object if it exists
                            Scene::current_scene = LoadScene(last_scene_path, asset_browser.GetAssetPath());
                        }
                    }
				}

				static int fps = 30;
				static bool fps_unlocked = false;
				ImGui::BeginDisabled(fps_unlocked);
				if (ImGui::DragInt("FPS", &fps, 1, 10, 100000))
				{
					Toad::GetWindow().setFramerateLimit(std::clamp(fps, 10, 100000));
				}
				ImGui::EndDisabled();
				ImGui::SameLine();
				if (ImGui::Checkbox("Unlock", &fps_unlocked))
				{
					if (fps_unlocked)
					{
						Toad::GetWindow().setFramerateLimit(0);
					}
					else
					{
						Toad::GetWindow().setFramerateLimit(fps);
					}
				}

				ImGui::Checkbox("Reload scene on stop", &reload_scene_on_stop);

				if (ImGui::TreeNode("Editor Camera Settings"))
				{
					Vec2f pos = editor_cam.GetPosition();
					Vec2f size = editor_cam.GetSize();
					if (ImGui::SliderVec2("pos", &pos, FLT_MIN, FLT_MAX))
						editor_cam.SetPosition(pos);
					if (ImGui::SliderVec2("size", &size, FLT_MIN, FLT_MAX))
						editor_cam.SetSize(size);
					ImGui::TreePop();
				}

				ImGui::Checkbox("Show grid", &show_grid);
				ImGui::Checkbox("Snapping", &drag_snap);
				ImGui::SliderVec2i("Grid", &grid_size);

				ImGui::TreePop();
			}

			static std::vector<TileSpritePlacer> opened_tilemaps;
			if (ImGui::TreeNode("Tools"))
			{
				if (ImGui::TreeNode("open tilemaps"))
				{
					ImGui::Button("open (draggable by file)");
					if (ImGui::BeginDragDropTarget())
					{
						const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("move file");
						if (payload != nullptr)
						{
							std::filesystem::path src = *(std::string*)payload->Data;
							do
							{
								if (!src.has_extension() || (src.extension().string() != ".jpg" && src.extension().string() != ".png"))
								{
									LOGERRORF("[UI:Tools:open tilemaps] {} is not a valid file for a tilemap", src);
									break;
								}
								for (const auto& t : opened_tilemaps)
								{
									if (t.path == src)
									{
										LOGDEBUGF("[UI:Tools:open tilemaps] {} already is an existing tilemap", src);
										break;
									}
								}

								sf::Texture tex;
								if (!tex.loadFromFile(src.string()))
								{
									LOGDEBUGF("[UI:Tools:open tilemaps] {} can't be opened/loaded", src);
									break;
								}
								opened_tilemaps.push_back(TileSpritePlacer(src, { 16, 16 }, tex, {}));
							} while (false);
						}

						ImGui::EndDragDropTarget();
					}
					for (auto it = opened_tilemaps.begin(); it != opened_tilemaps.end();)
					{
						ImGui::Text("%s", it->path.filename().string().c_str());
						ImGui::SameLine();
						if (ImGui::Button("close"))
						{
							it = opened_tilemaps.erase(it);
						}
						else
						{
							++it;
						}
					}

					ImGui::TreePop();
				}

				ImGui::TreePop();
			}

			for (auto& t : opened_tilemaps)
			{
				ImGui::Begin(t.path.filename().string().c_str());
				{
					static bool ignore_fully_transparent = true;
					static bool preview_size = true;
					static Vec2f tile_size = { 1.f, 1.f };
					ImGui::SliderVec2i("split size", &t.size);
					ImGui::SliderVec2("tile dropped size", &tile_size);

					ImGui::Checkbox("ignore full transparent tiles", &ignore_fully_transparent);
					ImGui::Checkbox("preview size", &preview_size);
					if (ImGui::Button("APPLY SIZE/RESET TEXTURE"))
					{
						assert(t.size.x < t.tile_map.getSize().x && t.size.y < t.tile_map.getSize().y);

						t.tiles.clear();

						for (uint32_t i = 0; i < t.tile_map.getSize().x; i += t.size.x)
						{
							for (uint32_t j = 0; j < t.tile_map.getSize().y; j += t.size.y)
							{
								sf::Sprite sprite = sf::Sprite(t.tile_map);
								sprite.setTextureRect(sf::IntRect({ (int)i, (int)j }, t.size));

								bool skip = false;

								if (ignore_fully_transparent)
								{
									skip = true;
									// must copy to an image 
									auto image = t.tile_map.copyToImage();

									for (uint32_t sprite_x = i; sprite_x < i + t.size.x; sprite_x++)
									{
										for (uint32_t sprite_y = j; sprite_y < j + t.size.y; sprite_y++)
										{
											if (image.getPixel({sprite_x, sprite_y}) != sf::Color::Transparent)
											{
												skip = false;
											}
										}
									}
								}

								if (!skip)
								{
									t.tiles.emplace_back(sprite);
								}
							}
						}
					}
					if (ImGui::TreeNode("preview"))
					{
						t.size.x = std::clamp(t.size.x, 5, INT_MAX);
						t.size.y = std::clamp(t.size.y, 5, INT_MAX);

						ImVec2 screen_pos = ImGui::GetCursorScreenPos();
						ImGui::Image(t.tile_map);
						if (preview_size)
						{
							auto draw = ImGui::GetWindowDrawList();
							for (uint32_t x = 0; x < t.tile_map.getSize().x; x += t.size.x)
							{
								for (uint32_t y = 0; y < t.tile_map.getSize().y; y += t.size.y)
								{
									draw->AddLine(
										screen_pos + ImVec2{ (float)x, (float)y },
										screen_pos + ImVec2{ (float)x, (float)t.tile_map.getSize().y },
										IM_COL32(255, 255, 255, 60)
									);
									draw->AddLine(
										screen_pos + ImVec2{ (float)x, (float)y },
										screen_pos + ImVec2{ (float)t.tile_map.getSize().x, (float)y },
										IM_COL32(255, 255, 255, 60)
									);
								}
							}
						}
						ImGui::TreePop();
					}
					if (ImGui::TreeNode("drag & drop"))
					{
						float y = ImGui::GetCursorPosY();
						uint32_t total_h = 0;

						// t.tiles is saved vertically 
						// split them up and store them in vertical lines for proper display
						std::vector<std::vector<const sf::Sprite*>> splitted_tiles;
						std::vector <const sf::Sprite*> line;

						for (const auto& t2 : t.tiles)
						{
							total_h += t.size.y;
							line.push_back(&t2);
							if (total_h > t.tile_map.getSize().y)
							{
								splitted_tiles.push_back(line);
								line.clear();
								total_h = 0;
							}
						}
						for (uint32_t i = 0; i < splitted_tiles.size(); i++)
						{
							ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });
							ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0, 0 });
							ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
							ImGui::BeginChild(format_str("{}", i).c_str(), { (float)t.size.x, ImGui::GetWindowHeight() - y }, false, ImGuiWindowFlags_NoScrollbar);
							for (const sf::Sprite* sprite : splitted_tiles[i])
							{
								ImGui::Image(*sprite);
								if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
								{
									std::filesystem::path relative = std::filesystem::relative(t.path, asset_browser.GetAssetPath());

									if (!ResourceManager::GetTextures().Get(relative.string()))
									{
										ResourceManager::GetTextures().Add(relative.string(), t.tile_map);
									}

									SheetTileData data;
									data.path = new char[relative.string().length()];
									strncpy(data.path, relative.string().c_str(), relative.string().length() + 1);
									data.tex_rect = sprite->getTextureRect();
									data.tex_size = tile_size;
									ImGui::SetDragDropPayload("dnd tilesheettile", &data, sizeof(data));
									ImGui::EndDragDropSource();
								}

							}
							ImGui::EndChild();
							ImGui::PopStyleVar(3);

							if (i < splitted_tiles.size() - 1)
							{
								ImGui::SameLine(0, 0);
							}
						}

						ImGui::TreePop();
					}
					ImGui::End();
				}
			}

			ImGui::End();
		}

	}

}
