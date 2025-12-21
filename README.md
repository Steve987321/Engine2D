Engine2D
========

Engine2D is a game engine to create 2D games/applications in. It currently supports Mac and Windows.

This project uses [SFML](https://www.sfml-dev.org/) for rendering, audio and window handling.

## How to setup

***This setup guide might contain outdated information.***

The steps below will show how to get the engine compiled and running:

### Windows

*It is recommended to use Visual Studio 2022 to build and run the engine*

1. Clone the repo.

2. If Python is installed, go to the `scripts/` folder and execute `setup.bat`.

   If Python is not installed then execute `generate_projectfiles_for_vs2022.bat` and manually copy the correct sfml dlls after building. 

3. After generating the project files, open the project into Visual Studio by opening the `Onion.sln` file.

    Set the solution configuration to **Release** and the solution platform to **x64**.

    Then right click on the solution and select **Build Solution** or press **CTRL+SHIFT+B**.

5. After building finishes, run the editor from Visual Studio by setting the startup project to **ToadRunner** and starting a debug session. 

### Mac

1. Clone the repo.

1. Ensure Python is installed, then run the `scripts/setup.command` file. 
  
1. Run `make` (optionally with -j for faster builds) to build for debug configuration.

1. Execute ToadRunner.

Generating for XCode can be done using the xcode4 option when using premake.

## How to use
*The engine is still very undocumented and unstable in some areas but is stable enough to create 2D applications in which can be packaged from the engine and shared.*

The steps below will show the minimum steps required on how to create an example application and package it.

1. **Creating the project files inside the editor**

    In the editor go to the top menu bar and select **Engine** -> **Create Project..**

    This will open a popup where you need to fill in:
    * The name of the project
    * The project file type
    * The project directory
    * The project template you want to use

    The engine path text refers to the engine **project** not the main project path.
    For a distro version of the engine this will just be where the engine executable is located.
  
    Press **Create**. 

    The created project directory tree should look something like this:

    <pre>
    ├── GameName_Game/            # Main game project, includes all the game assets and scripts.
    ├── vendor/                   # Third-party libraries such as sfml, imgui, etc.
    ├── GameName.sln              # The generated project file, which depends on the selected project file type.
    ├── GameName.TOADPROJECT      # The engine project file which contains name, application settings, etc.
    ├── premake5.exe              # Project file generator (see https://premake.github.io/).
    ├── premake5.lua              # Used for generating/updating the project files. Using the editors Update Project option reruns this.
    </pre>

    The project has also been automatically loaded into the editor. On Windows you can press **open .sln** in the asset browser window to open the project in visual studio.

1. **Creating a Scene** 

    In the asset browser, right click and create a scene file.
    
    Name it StartScene, the name will make sure that this scene will be the entry scene of the application. 

    Double click on the scene file to load it. 
    
    In the scene hierarchy right click and create a Camera and a Circle.

    Pressing Command/Control + S will save the scene that is being edited to the corresponding file.

1. **Creating a C++ script**

    To add a game script, in the asset browser, right click and select Create C++ Script. This will add a .cpp and .h file containing the default structure. 

    If your project is open in Visual Studio, it will prompt you to reload the solution. 

    A header file can look like this: 
    ```c++
    #pragma once

    class GAME_NAME_API ExampleScript : public Toad::Script
    {
    public:
        SCRIPT_CONSTRUCT(ExampleScript)

        float start_direction_x = 1;
        float start_direction_y = 1;
        float speed_mult = 1;

        // Called on scene start
        void OnStart(Toad::Object* obj) override;

        // Called every frame 
        void OnUpdate(Toad::Object* obj) override;

        // Called every fixed tick 
        void OnFixedUpdate(Toad::Object* obj) override;

        // Call EXPOSE_VAR(X) here, exposes variable to the scene file 
        void ExposeVars() override;

    #ifdef TOAD_EDITOR
        // Called in the inspector 
        void OnEditorUI(Toad::Object* obj, const UICtx& ctx) override;
    #endif
    #if defined(TOAD_EDITOR) || !defined(NDEBUG)
        // Write imgui code here, for in the editor and debug builds
        void OnImGui(Toad::Object* obj, const UICtx& ctx) override;
    #endif 

    private:
        float vel_x = 1;
        float vel_y = 1;

        Toad::Circle* circle = nullptr; // change the color if script is attached on a circle 
    };
    ```

    Most functions in the class will have their first parameter be the Object that the script is attached to.
    An Object is the base class for all types of objects in a scene. 

    Example implementation: 
    ```c++
    #include "framework/Framework.h" // Includes important headers 
    #include "ExampleScript.h"

    #include "engine/Engine.h" // Engine function declarations 

    using namespace Toad; // Main engine namespace 

    void ExampleScript::OnStart(Object* obj)
    {
        Script::OnStart(obj);

        vel_x = start_direction_x;
        vel_y = start_direction_y;

        // this can return nullptr if the object can't be converted to the specified type
        circle = GetObjectAsType<Circle>(obj);
    }

    void ExampleScript::OnUpdate(Object* obj)
    {
        Script::OnUpdate(obj);
    }

    void ExampleScript::OnFixedUpdate(Object* obj)
    {
	    const auto change_c_col = [](Circle* circle)
		{
            Color new_color(
                rand_int(0, 255), 
                rand_int(0, 255), 
                rand_int(0, 255));

			circle->SetFillColor(new_color);
		};

        FloatRect bounds = obj->GetBounds();

        // get the camera this is currently being used (aka interacting camera)
        // in the editor this can be either the viewport cam or the game cam depending which one is currently being interacted with
        Camera* active_cam = Camera::GetActiveCamera(); 

        if (active_cam)
        {
            // campos is the center of the camera 
            const Vec2f& campos = active_cam->GetPosition();
            const Vec2f& camsize = active_cam->GetSize();

            // get the left top of the camera using the center position and size of the camera 
            Vec2f cam_pos_left_top{ campos.x - camsize.x / 2.f, campos.y - camsize.y / 2.f };

            // simply check if the object is exceeding bounds and change velocity 
         	if (bounds.position.x < camposlefttop.x)
            {
                if (vel_x < 0)
                {
                    change_c_col();
                    vel_x *= -1;
                }
            }
            if (bounds.position.x + bounds.size.x > camposlefttop.x + camsize.x)
            {
                if (vel_x > 0)
                {
                    change_c_col();
                    vel_x *= -1;
                }
            }
            if (bounds.position.y < camposlefttop.y)
            {
                if (vel_y < 0)
                {
                    change_c_col();
                    vel_y *= -1;
                }
            }
            if (bounds.position.y + bounds.size.y > camposlefttop.y + camsize.y)
            {
                if (vel_y > 0)
                {
                    change_c_col();
                    vel_y *= -1;
                }
            }
        }

        Vec2f velocity{ vel_x, vel_y };

        // integrate the position using the velocity 
        obj->SetPosition(obj->GetPosition() + velocity * speed_mult * Time::GetFixedDeltaTime());
    }

    void ExampleScript::ExposeVars()
    {
        Script::ExposeVars();

        EXPOSE_VAR(start_direction_X);
        EXPOSE_VAR(start_direction_Y);
        EXPOSE_VAR(speed_mult);
    }

    #ifdef TOAD_EDITOR
    void ExampleScript::OnEditorUI(Toad::Object* obj, ImGuiContext* ctx)
    {
        // because we are working with DLL files we need to share the imgui state between modules
        ImGui::SetCurrentContext(ctx);

        ImGui::Text("Velocity: %f %f", vel_x, vel_y);
    }
    #endif

    #if defined(TOAD_EDITOR) || !defined(NDEBUG)
    void ExampleScript::OnImGui(Object* obj, ImGuiContext* ctx)
    {
        // because we are working with DLL files we need to share the imgui state between modules
        ImGui::SetCurrentContext(ctx);

        ImGui::Begin("[ExampleScript] Controller menu");

        static float fixed_time_step = Time::GetFixedDeltaTime();
        static int fps = 30;

        if (ImGui::DragInt("FPS Lock", &fps))
            GetWindow().setFramerateLimit(fps);
        
        if (ImGui::InputFloat("Fixed Step Delta", &fixed_time_step, 1.0e-7f, 5.f, "%.7f"))
            Time::SetFixedDeltaTime(std::clamp(fixed_time_step, 1.0e-7f, 5.f));

        ImGui::DragFloat("Ball Speed Mult", &speed_mult);

        ImGui::SeparatorText("Info");
            
        ImGui::Text("Frame per sec: %f", 1.f / Time::GetDeltaTime());

        ImGui::End();
    }
    #endif 
    ```

    To apply script changes: 
      - **Windows**: In Visual Studio build the project using the **Dev** (or **DevDebug** if engine is in debug config) configuration. 
      - On other systems use `make config=dev` or `config=devdebug`.

    The engine will notify once scripts have been updated and will automatically update objects that have new scripts.
    During an active play session, it will reload and restart the scene automatically.  

    To reload manually, press **Scripts** -> **Reload** 
   
1. **Attaching scripts on objects**

    Click on an object. In the inspector under object properties right click on Scripts and select the script to add. 

1. **Packaging**

    Go to Engine->Package, select an output directory and press Create. It will build and export a shareable version.
   
## Others 

Descriptions for some systems in the engine:

### FSM

..

### ScriptManager

..

### TileSheetDropper

..

### Flow

..
  
## Disclaimer
This project was primarily created for learning and experimentation purposes.

## Screenshots 

![image](https://github.com/user-attachments/assets/40381fce-eade-4056-837a-4fc738c69b72)

![image](https://github.com/user-attachments/assets/7762c24d-f58e-424b-b439-2513544f643a)

![image](https://github.com/user-attachments/assets/dd46f57f-c384-491b-95a2-834e52ac1c39)


