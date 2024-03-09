--premake5.lua

newoption{
    trigger = "enginepath",
    description = "Set the engine installation path"
}

newoption{
    trigger = "projectname",
    description = "The name of the game project"
}

newoption{
    trigger = "usesrc",
    description = "Set if using a version of the engine that has the source code"
}

if not _OPTIONS["enginepath"] then
error("Error: The --enginepath argument is required.")
end

if not _OPTIONS["projectname"] then
error("Error: The --projectname argument is required.")
end

output_dir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
engine_path = _OPTIONS["enginepath"]
game_project_name = _OPTIONS["projectname"] .. "_Game"

workspace(_OPTIONS["projectname"])
    architecture "x64"
    configurations{
        "Release",
        "Dev", -- same as release but defines TOAD_EDITOR
        "DevDebug", -- with debugging symbols and debugging runtime library
    }
    
    startproject (game_project_name)

if _OPTIONS["usesrc"] then 
project "Engine"
    location "Engine"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"

    targetdir("bin/" ..output_dir .. "/")
    objdir("bin-intermediate/" ..output_dir .. "/")

    files{
        engine_path .. "/src/**.cpp",
        engine_path .. "/src/**.h"
    }

    includedirs{
        engine_path .. "/src",
        game_project_name .. "/src",
        "vendor",
        "vendor/imgui",
        "vendor/sfml-imgui",
        "vendor/SFML-2.6.0/include",
        "vendor/json/include"
    }

    libdirs{
        "vendor/SFML-2.6.0/lib"
    }

    -- configurations 

    filter "configurations:Release"
        defines{
            "NDEBUG",
            "_CONSOLE",
            "ENGINE_IS_EXPORT"
        }
        links{
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }

        staticruntime "On"
        runtime "Release"
        optimize "On"
        symbols "Off"

    filter "configurations:Dev"
        defines{
            "NDEBUG",
            "_CONSOLE",
            "ENGINE_IS_EXPORT",
            "TOAD_EDITOR"
        }
        links{
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }

        staticruntime "On"
        runtime "Release"
        optimize "On"
        symbols "Off"
       
    filter "configurations:DevDebug"
        defines{
            "_DEBUG",
            "_CONSOLE",
            "ENGINE_IS_EXPORT",
            "TOAD_EDITOR"
        }

        staticruntime "On"
        runtime "Debug"
        optimize "Off"
        symbols "On"

        filter "system:macosx"
            links{
                "sfml-system",
                "sfml-window",
                "sfml-graphics",
                "sfml-audio",
            }
        filter "system:windows"
            links{
                "sfml-system-d",
                "sfml-window-d",
                "sfml-graphics-d",
                "sfml-audio-d",
            }

    -- platform 

    filter "system:macosx"
         links {
            "OpenGL.framework",
            "Cocoa.framework",
            "IOKit.framework",
            "CoreVideo.framework",
         }

    filter "system:windows"
        links {
            "opengl32"
        }

        staticruntime "On"
        systemversion "latest"
end 

project(game_project_name)
    location(game_project_name)
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"

    targetdir("bin/" ..output_dir .. "/")
    objdir("bin-intermediate/" ..output_dir .. "/")

    files{
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h"
    }

if _OPTIONS["usesrc"] then 
    includedirs{
        engine_path .. "/script_api",
        "vendor",
        "vendor/imgui",
        "vendor/SFML-2.6.0/include",
        "vendor/json/include",
        "%{prj.name}/src",
        "%{prj.name}/src/scripts"
    }
    libdirs{
        engine_path .. "/libs"
    }
else
    includedirs{
        engine_path .. "/src",
        "vendor",
        "vendor/imgui",
        "vendor/SFML-2.6.0/include",
        "vendor/json/include",
        "%{prj.name}/src",
        "%{prj.name}/src/scripts"
    }
    libdirs{
        "vendor/SFML-2.6.0/lib"
    }
end 

    staticruntime "Off"
    -- configurations

    filter "configurations:Release"
        defines{
            "NDEBUG",
            "_WINDOWS",
            "_USRDLL",
            "GAME_IS_EXPORT"
        }
        links{
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }

        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "configurations:Dev"
        defines{
            "NDEBUG",
            "_WINDOWS",
            "_USRDLL",
            "GAME_IS_EXPORT",
            "TOAD_EDITOR",
        }
        links{
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }

        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "configurations:DevDebug"
        defines{
            "_DEBUG",
            "_WINDOWS",
            "GAME_IS_EXPORT",
            "TOAD_EDITOR",
        }

        runtime "Debug"
        symbols "On"

        filter "system:macosx"
            links{
                "sfml-system",
                "sfml-window",
                "sfml-graphics",
                "sfml-audio",
            }
        filter "system:windows"
            links{
                "sfml-system-d",
                "sfml-window-d",
                "sfml-graphics-d",
                "sfml-audio-d",
            }
       
    -- platform 

    filter "system:macosx"
        links {
            "Engine",
            "OpenGL.framework",
            "Cocoa.framework",
            "IOKit.framework",
            "CoreVideo.framework",
        }

    filter "system:windows"
        links {
            "Engine",
            "opengl32"
        }

        systemversion "latest"

if _OPTIONS["usesrc"] then 
project "ToadRunner"
    location "ToadRunner"
    kind "ConsoleApp"
    language "C++"

    targetdir("bin/" ..output_dir .. "/")
    objdir("bin-intermediate/" ..output_dir .. "/")

    files{
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h"
    }

    includedirs{
        "%{prj.name}/src",
        "%{prj.name}/src/scripts",
        engine_path .. "/src",
        "vendor",
        "vendor/imgui",
        "vendor/sfml-imgui",
        "vendor/SFML-2.6.0/include",
        "vendor/json/include",
    }

    libdirs{
        "vendor/SFML-2.6.0/lib"
    }

    filter "system:macosx"
         links {
                "Engine",
                "OpenGL.framework",
                "Cocoa.framework",
                "IOKit.framework",
                "CoreVideo.framework",
        }
    filter "system:windows"
        links {
            "Engine",
            "opengl32"
        }

    cppdialect "C++20"

    filter "system:windows"
        cppdialect "C++20"
        staticruntime "On"
        systemversion "latest"

    filter "configurations:Release"
        defines{
            "NDEBUG",
            "_CONSOLE",
        }
        links{
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }

        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "configurations:Dev or configurations:DevDebug"
        defines{
            "_DEBUG",
            "_CONSOLE",
            "TOAD_EDITOR"
        }

        filter "system:macosx"
            links {
                "sfml-system",
                "sfml-window",
                "sfml-graphics",
                "sfml-audio",
            }
        filter "system:windows"
            links {
                "sfml-system-d",
                "sfml-window-d",
                "sfml-graphics-d",
                "sfml-audio-d",
            }
            
        symbols "On"
        runtime "Debug"
end 