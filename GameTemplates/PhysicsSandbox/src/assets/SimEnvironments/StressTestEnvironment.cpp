#include "framework/Framework.h"
#include "StressTestEnvironment.h"

#include "scripts/Sim.h"
#include "Fizzix/FZWorld.h"
#include "Fizzix/FZMath.h"

namespace SimEnvironments
{
    static float time = 0;
    static std::array<Toad::Vec2f, 6> square_vertices;
    static std::array<float, 25> fps_data;

    void StressTestEnvironmentLoad(size_t count)
    {
        fz::World& world = Sim::GetWorld();

        time = 0;
        square_vertices = fz::CreateSquare(20, 20);

        world.polygons.clear();
        world.springs.clear();
        
        std::array<Toad::Vec2f, 6> floor_vertices = fz::CreateSquare(1050, 250);
        std::array<Toad::Vec2f, 6> wall_vertices = fz::CreateSquare(50, 1000);

        fz::Polygon floor({floor_vertices.begin(), floor_vertices.end()});
        floor.rb.inv_mass = 1.f / 1000.f;
        floor.rb.is_static = true;
        floor.Translate({-500, 0});

        fz::Polygon wall_left({wall_vertices.begin(), wall_vertices.end()});
        wall_left.rb.inv_mass = 1.f / 1000.f;
        wall_left.rb.is_static = true;
        wall_left.Translate({-500, -1000});

        fz::Polygon wall_right({wall_vertices.begin(), wall_vertices.end()});
        wall_right.rb.inv_mass = 1.f / 1000.f;
        wall_right.rb.is_static = true;
        wall_right.Translate({500, -1000});

        world.AddPolygon(floor);
        world.AddPolygon(wall_right);
        world.AddPolygon(wall_left);
    }

    void StressTestImGui(Sim &sim_script)
    {
        fz::World& s = sim_script.GetWorld();
        ImGui::Begin("[world] stress test menu");
        static bool shoot = false;
        ImGui::Checkbox("shoot", &shoot);
        float dt = Toad::Time::GetDeltaTime();
        float fps = 1.f / dt;
        static size_t i = 0;
        fps_data[i] = fps;
        i = (i + 1) % fps_data.size();
        fps = 0;
        for (float v : fps_data)
            fps += v;
        fps /= fps_data.size();

        // float min_fps = *std::min(fps_data.begin(), fps_data.end());
        const float fps_threshold = 400.f;
        float g = std::min(1.f, fps / fps_threshold);
        ImGui::TextColored({1.f - g, g, 0, 1}, "FPS %.1f", fps);
        // ImGui::TextColored({0, std::max(1.f, min_fps / fps_threshold), 0, 1}, "FPS min %.1f", min_fps);
        static int polygon_add_counter = 0;
        static int shoot_limit = 0;
        ImGui::Text("%d", polygon_add_counter);

        static float mass = 25.f;
        static float friction = 0.5f;
        static float shoot_delay = 0.1f;
        static float moi = 10000.f;
        static bool set_moi = false;
        static int grid_pos_offx = -400;
        static int grid_pos_offy = -800;
        static int grid_size_x = 10;
        static int grid_size_y = 10;
        static int grid_space_x = 10;
        static int grid_space_y = 10;
        
        ImGui::DragFloat("mass", &mass);
        ImGui::SliderFloat("friction", &friction, 0, 1);
        ImGui::DragFloat("shoot delay", &shoot_delay, 0.05f);
        ImGui::DragFloat("moi", &moi, 10.f);
        ImGui::Checkbox("set moi", &set_moi);
        ImGui::DragInt("shoot limit", &shoot_limit);
        ImGui::DragInt("grid posoffx", &grid_pos_offx);
        ImGui::DragInt("grid posoffy", &grid_pos_offy);
        ImGui::SliderInt("grid x", &grid_size_x, 5, 50);
        ImGui::SliderInt("grid y", &grid_size_y, 5, 50);
        ImGui::SliderInt("grid space x", &grid_space_x, 1, 10);
        ImGui::SliderInt("grid space y", &grid_space_y, 1, 10);

        if (shoot)
        {
            static int x = 0; 
            static int y = 0;
            
            time += dt;
            if (time > shoot_delay)
            {
                fz::Polygon p({square_vertices.begin(), square_vertices.end()});
                p.Translate({(float)x * 20 + grid_space_x + grid_pos_offx, (float)y * 20 + grid_space_y + grid_pos_offy});
                x++;
                if (x == grid_size_x)
                {
                    x = 0;
                    y++;
                }
                    
                // float r = ImSin(ImGui::GetTime() * 1.5f);
                // p.rb.velocity.y = 50.f;
                // p.rb.velocity.x = r * 100.f;
                p.rb.inv_mass = 1.f / mass;
                p.rb.friction = friction;
                if (set_moi)
                    p.rb.moment_of_inertia = moi;
                s.AddPolygon(p);
                time = 0;
                polygon_add_counter++;
            }

            if (polygon_add_counter > shoot_limit)
            {
                shoot = false;
                time = 0;
                polygon_add_counter = 0;
                x = 0;
                y = 0;
            }
            // if (fps < fps_threshold)
            //     shoot = false;
        }

        ImGui::End();
    }
}