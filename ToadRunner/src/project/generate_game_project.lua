-- premake5.lua

newoption {
    trigger = "enginepath",
    description = "set the main engine PROJECT path"
}

newoption {
    trigger = "projectname",
    description = "The name of the game project"
}

if not _OPTIONS["enginepath"] then 
    error("Error: The --enginepath argument is required.")
end 

if not _OPTIONS["projectname"] then 
    error("Error: The --projectname argument is required.")
end 

workspace (_OPTIONS["projectname"])
    architecture "x64"
    configurations { 
        "Debug", 
        "Release", 
        "Dev"
    }
    
output_dir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
engine_path = _OPTIONS["enginepath"]
game_project_name = _OPTIONS["projectname"] .. "_Game"

project "Engine"
    location "Engine"
    kind "SharedLib"
    language "C++"

    targetdir ("bin/" .. output_dir .. "/")
    objdir ("bin-intermediate/" .. output_dir .. "/")

    files {
        engine_path .. "/src/**.cpp",
        engine_path .. "/src/**.h"
    }

    includedirs {
        engine_path .. "/src",
        game_project_name .. "/src",
        "vendor",
        "vendor/imgui",
        "vendor/sfml-imgui",
        "vendor/SFML-2.6.0/include",
        "vendor/json/include"
    }

    libdirs {
        "vendor/SFML-2.6.0/lib"
    }

    links {
        "opengl32",
    }

    cppdialect "C++20"

    filter "system:windows"
        cppdialect "C++20"
        staticruntime "On"
        systemversion "latest"

    filter "configurations:Debug" 
        defines {
            "_DEBUG",
            "_CONSOLE",
            "ENGINE_IS_EXPORT",
        }
        
        links {
            "sfml-system-d",
            "sfml-window-d",
            "sfml-graphics-d",
            "sfml-audio-d",
        }
        symbols "On"
    
    filter "configurations:Release" 
        defines {
            "NDEBUG",
            "_CONSOLE",
            "ENGINE_IS_EXPORT"
        }
        links {
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }
        optimize "On"
    
    filter "configurations:Dev" 
        defines {
            "_DEBUG",
            "_CONSOLE",
            "ENGINE_IS_EXPORT",
            "TOAD_EDITOR"
        }
        links {
            "sfml-system-d",
            "sfml-window-d",
            "sfml-graphics-d",
            "sfml-audio-d",
        }
        symbols "On"
    
project (game_project_name)
    location (game_project_name)
    kind "SharedLib"
    language "C++"

    targetdir ("bin/" .. output_dir .. "/")
    objdir ("bin-intermediate/" .. output_dir .. "/")

    files {
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h"
    }

    includedirs {
        engine_path .. "/src",
        "vendor",
        "vendor/SFML-2.6.0/include",
        "%{prj.name}/src",
        "%{prj.name}/src/scripts"
    }

    libdirs {
        "vendor/SFML-2.6.0/lib"
    }

    links {
        "Engine",
        "opengl32",
    }

    cppdialect "C++20"

    filter "system:windows"
        cppdialect "C++20"
        staticruntime "On"
        systemversion "latest"

    filter "configurations:Debug" 
        defines {
            "_DEBUG",
            "_WINDOWS",
            "_USRDLL"
        }
        links {
            "sfml-system-d",
            "sfml-window-d",
            "sfml-graphics-d",
            "sfml-audio-d",
        }
        symbols "On"
    
    filter "configurations:Release" 
        defines {
            "NDEBUG",
            "_WINDOWS",
            "_USRDLL",
            "GAME_IS_EXPORT"
        }
        links {
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }
        optimize "On"
    
    filter "configurations:Dev" 
        defines {
            "_DEBUG",
            "_WINDOWS",
            "GAME_IS_EXPORT",
            "TOAD_EDITOR",
            "_USRDLL"
        }
        links {
            "sfml-system-d",
            "sfml-window-d",
            "sfml-graphics-d",
            "sfml-audio-d",
        }
        symbols "On"

project "ToadRunner"
    location "ToadRunner"
    kind "ConsoleApp"
    language "C++"

    targetdir ("bin/" .. output_dir .. "/")
    objdir ("bin-intermediate/" .. output_dir .. "/")

    files {
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h"
    }

    includedirs {
        "%{prj.name}/src",
        "%{prj.name}/src/scripts",
        engine_path .. "/src",
        "vendor",
        "vendor/imgui",
        "vendor/sfml-imgui",
        "vendor/SFML-2.6.0/include",
        "vendor/json/include",
    }

    libdirs {
        "vendor/SFML-2.6.0/lib"
    }

    links {
        "Engine",
        "Game1_Game",
        "opengl32",
    }

    cppdialect "C++20"

    filter "system:windows"
        cppdialect "C++20"
        staticruntime "On"
        systemversion "latest"

    filter "configurations:Debug" 
        defines {
            "_DEBUG",
            "_CONSOLE",
        }
        links {
            "sfml-system-d",
            "sfml-window-d",
            "sfml-graphics-d",
            "sfml-audio-d",
        }
        symbols "On"

    filter "configurations:Release" 
        defines {
            "NDEBUG",
            "_CONSOLE",
        }
        links {
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }
        optimize "On"

    filter "configurations:Dev" 
        defines {
            "_DEBUG",
            "_CONSOLE",
            "TOAD_EDITOR"
        }
        links {
            "sfml-system-d",
            "sfml-window-d",
            "sfml-graphics-d",
            "sfml-audio-d",
        }
        symbols "On"
        