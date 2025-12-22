#if defined(TOAD_EDITOR) || !defined(NDEBUG)
#include "framework/Framework.h"
#include "Fizzix/FZWorld.h"
#include "SimEnvironments/CarEnvironment.h"
#include "SimEnvironments/StressTestEnvironment.h"
#include "SimEnvironments/SandBoxEnvironment.h"
#include "SimEnvironments/RocketEnvironment.h"
#include "scripts/Sim.h"

#include "implot/implot.h"

namespace UI
{
    constexpr static size_t history_size = 100;

    struct ThrusterData
    {
        float thrust_history[history_size] = {};
        size_t i = 0;
    };

    struct SpringData
    {
        float energy_history[history_size];
        size_t i = 0;
        void AddEnergy(float e)
        {
            for (int j = 0; j < history_size - 1; j++)
            {
                energy_history[j] = energy_history[j + 1];
            }

            energy_history[history_size - 1] = e;
        }
    };

    static std::vector<ThrusterData> thrusters_data;
    static std::vector<SpringData> springs_data;
    
    static bool show_thruster_graphs = false; 
    static bool show_spring_graphs = false; 

    void FizzixMenu(Sim& sim_script, char* source)
    { 
        using namespace Toad;

        ImGui::Begin("[Sim] fizzix menu");

        fz::World& world = sim_script.GetWorld();

        if (ImGui::Button("Load CarScene"))
        {
            sim_script.env_car_loaded = true;
            sim_script.env_stress_test_loaded = false;
            SimEnvironments::CarEnvironmentLoad();

            // copy default script to source 
            strncpy(source, SimEnvironments::car_controller_script, strlen(SimEnvironments::car_controller_script) + 1);
        }
        if (ImGui::Button("Load StressTest"))
        {
            sim_script.env_car_loaded = false;
            sim_script.env_stress_test_loaded = true;
            SimEnvironments::StressTestEnvironmentLoad(0);
        }

        if (ImGui::Button("Load Sandbox"))
        {
            sim_script.env_car_loaded = false;
            sim_script.env_stress_test_loaded = false;
            SimEnvironments::SandBoxEnvironmetLoad();
        }

        if (ImGui::Button("Load RocketScene"))
        {
            sim_script.env_car_loaded = false;
            sim_script.env_stress_test_loaded = false;
            SimEnvironments::RocketEnvironmentLoad();
        }

        ImGui::Checkbox("Show Thruster Graphs", &show_thruster_graphs);
        ImGui::Checkbox("Show Spring Energy Graphs", &show_spring_graphs);

        // if (show_thruster_graphs)
        // {
        //     ImGui::Begin("thruster graphs");

        //     thrusters_data.resize(world.thrusters.size());

        //     for (size_t i = 0; i < world.thrusters.size(); i++)
        //     {
        //         const fz::Thruster& t = world.thrusters[i];
        //         if (ImPlot::BeginPlot(std::to_string(t.id).c_str())) 
        //         {
        //             ImPlot::PlotLine("thrust", thrusters_data[i].thrust_history, 100);
        //             ImPlot::EndPlot();
        //         }
        //     }   
        

        //     ImGui:End();
        // }

        if (show_spring_graphs)
        {
            ImGui::Begin("[Sim] spring graphs");

            static float zoom = 1.5f;
            static float zoom_inv = 1.f / zoom;
            static bool show_target_line = true;
            ImGui::Text("%zu", world.springs.size());
            if (ImGui::SliderFloat("zoom factor", &zoom, 0.1f, 2.f))
                zoom_inv = 1.f / zoom;

            ImGui::Checkbox("show target line", &show_target_line);

            // ensure same size 
            springs_data.resize(world.springs.size());

            ImPlotFlags plot_flags = ImPlotFlags_NoLegend | ImPlotFlags_NoTitle;
            ImPlotAxisFlags axis_flags = ImPlotAxisFlags_NoDecorations;

            for (size_t i = 0; i < world.springs.size(); i++)
            {
                ImGui::Text("spring (%zu)", i);
                SpringData& data = springs_data[i];
                const fz::Spring& s = world.springs[i];
                data.AddEnergy(s.len);
                ImGui::PushID(i);
                if (ImPlot::BeginPlot("##springplot", {-1, 100}, plot_flags)) 
                {
                    ImPlot::SetupAxes(nullptr, nullptr, axis_flags, axis_flags);
                    ImPlot::SetupAxesLimits(0, 100, s.target_len * zoom_inv, s.target_len * zoom, ImGuiCond_Always);

                    if (show_target_line)
                    {
                        ImPlot::PushStyleColor(ImPlotCol_Line, {1, 1, 1, 0.3f});
                        ImPlot::PlotInfLines("##ref", &s.target_len, 1, ImPlotInfLinesFlags_Horizontal);
                        ImPlot::PopStyleColor();
                    }
                    
                    ImPlot::PlotLine("##dist", springs_data[i].energy_history, 100);
                    ImPlot::EndPlot();
                }
                ImGui::PopID();
            }   

            ImGui::End();
        }
        if (ImGui::Button("Clear"))
        {
            world = fz::World();
        }
        if (!world.polygons.empty())
        {
            if (ImGui::Button("FORCE"))
            {
                world.polygons[0].rb.velocity += Vec2f{0, -10.f};
            }
            if (ImGui::Button("ANGULARA"))
            {
                // Sim::env_car_gas = -10.f;
                world.polygons[1].rb.angular_velocity += 2.f;
                // world.polygons[1].rb.angular_velocity += 2.f;
            }if (ImGui::Button("ANGULARB"))
            {
                // Sim::env_car_gas = 10.f;
                world.polygons[1].rb.angular_velocity -= 2.f;
                // world.polygons[1].rb.angular_velocity -= 2.f;
            }

        }

        static float angle = 0;
        static float scale = 1.f;
        static float fdt = Time::GetFixedDeltaTime();
        ImGui::DragFloat("Angle", &angle);
        ImGui::Checkbox("Pause", &sim_script.pause_sim);
        ImGui::DragFloat("SimGravity Y", &world.gravity.y);
        ImGui::DragFloat("Time scale", &scale, 0.05f);
        ImGui::DragFloat("Fixed time step", &fdt, 0.01f);
        ImGui::Checkbox("Show AABB", &sim_script.show_aabb);
        ImGui::Checkbox("Show Velocities", &sim_script.show_velocities);
        
        float grav_edit[2] = {world.gravity.x, world.gravity.y};
        if (ImGui::DragFloat2("Grav", grav_edit, 0.1f))
        {	
            world.gravity.x = grav_edit[0];
            world.gravity.y = grav_edit[1];
        }
        if (ImGui::Button("Set time scale"))
            Time::SetTimeScale(scale);
        if (ImGui::Button("Set fixed DT"))
            Time::SetFixedDeltaTime(fdt);

        for (int i = 0; i < world.thrusters.size(); i++)
        {
            ImGui::PushID(i);

            if (ImGui::TreeNode("Thruster", "Thruster %d", i))
            {
                fz::Thruster& t = world.thrusters[i];

                ImGui::Text("attached: (%llu)", t.attached_polygon);
                ImGui::Text("thrust %f", t.power);
                ImGui::DragFloat("max thrust", &t.max_power);
                if (ImGui::DragFloat("direction (degrees)", &t.direction_deg))
                {
                    t.SetDirection(t.direction_deg);
                }
                
                ImGui::DragFloat("rel pos x", &t.attached_rel_pos.x);
                ImGui::DragFloat("rel pos y", &t.attached_rel_pos.y);

                ImGui::TreePop();
            }

            ImGui::PopID();
        }
        for (int i = 0; i < world.springs.size(); i++)
        {
            ImGui::PushID(i);

            if (ImGui::TreeNode("Spring", "Spring %d", i))
            {
                fz::Spring& spr = world.springs[i];

                ImGui::Text("attached A: (%.2f %.2f) B: (%.2f %.2f)", spr.start_rb->center.x,  spr.start_rb->center.y, spr.end_rb->center.x, spr.end_rb->center.y);
                
                ImGui::DragFloat("Stiffness", &spr.stiffness, 0.05f);
                ImGui::DragFloat("Rotational force factor", &spr.rotation_force_factor, 0.1f);
                ImGui::DragFloat("Rebound damping", &spr.rebound_damping, 0.05f);
                ImGui::DragFloat("Compression damping", &spr.compression_damping, 0.05f);
                ImGui::DragFloat("Target len", &spr.target_len);
                ImGui::DragFloat("Min len", &spr.min_len);
                
                ImGui::TreePop();
            }

            ImGui::PopID();
        }
        for (int i = 0; i < world.polygons.size(); i++)
        {
            ImGui::PushID(i);

            if (ImGui::TreeNode("Object", "Object %d", i))
            {
                fz::Rigidbody& rb = world.polygons[i].rb;

                ImGui::DragFloat("Moment of inertia", &rb.moment_of_inertia);
                ImGui::SliderFloat("Restitution", &rb.restitution, 0.0f, 1.f);
                
                if (ImGui::DragFloat("InvMass", &rb.inv_mass, 0.01f))
                    if (rb.inv_mass <= 0.0001f) 
                        rb.inv_mass = 0.0001f;
                        
                ImGui::SliderFloat("Friction", &rb.friction, 0.0f, 1.f);
                
                ImGui::Text("Center(%.2f, %.2f) Sleeping(%d) Slide(%.2f)", rb.center.x, rb.center.y, rb.is_sleeping, rb.slide);
                ImGui::Text("Angular(%.2f)", rb.angular_velocity);
                ImGui::Text("Vel(%.2f, %.2f) VelLength(%.3f)", rb.velocity.x, rb.velocity.y, rb.velocity.Length());
                for (auto& f : world.polygons[i].vertices)
                {
                    ImGui::Text("%.1f, %.1f", f.x, f.y);
                }
                if (ImGui::Button("Rotate"))
                {
                    world.polygons[i].Rotate(angle);
                }

                ImGui::TreePop();
            }

            ImGui::PopID();
        }

        ImGui::End();

        auto draw = ImGui::GetForegroundDrawList();

        draw->AddText({50.f, 120.f}, IM_COL32(255, 255, 0, 255), std::to_string(txt_to_draw.size()).c_str());
        int i = 10;
        for (const auto& [k, v] : txt_to_draw)
        {	
            draw->AddText({50.f, (float)i * 20.f}, IM_COL32(255, 255, 0, 255), v.c_str());
            i++;
        }

        if (!txt_to_draw.empty())
            txt_to_draw.clear();
        if (!rect_to_draw.empty())
            rect_to_draw.clear();
            
    }
}
#endif 