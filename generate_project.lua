-- premake5.lua

workspace "Onion"
    architecture "x64"
    configurations { 
        "Debug", 
        "Release", 
        "Dev"
    }
    
output_dir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Engine"
    location "Engine"
    kind "SharedLib"
    language "C++"

    targetdir ("bin/" .. output_dir .. "/%{prj.name}")
    objdir ("bin-intermediate/" .. output_dir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h"
    }

    includedirs {
        "%{prj.name}/src",
        "vendor",
        "vendor/sfml-imgui",
        "vendor/SFML-2.5.1/include"
    }

    filter "system:windows"
        cppdialect "C++20"
        staticruntime "On"
        systemversion "latest"

    filter "configurations:Debug" 
        defines {
            "_DEBUG",
            "_CONSOLE",
            "ENGINE_IS_EXPORT",
            "TOAD_EDITOR"
        }
        symbols "On"
    
    filter "configurations:Release" 
        defines {
            "NDEBUG",
            "_CONSOLE",
            "ENGINE_IS_EXPORT"
        }
        optimize "On"
    
    filter "configurations:Dev" 
        defines {
            "_DEBUG",
            "_CONSOLE",
            "ENGINE_IS_EXPORT",
            "TOAD_EDITOR"
        }
        symbols "On"
    
project "Game"
    location "Game"
    kind "SharedLib"
    language "C++"

    targetdir ("bin/" .. output_dir .. "/%{prj.name}")
    objdir ("bin-intermediate/" .. output_dir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h"
    }

    includedirs {
        "%{prj.name}/src",
        "vendor",
        "vendor/sfml-imgui",
        "vendor/SFML-2.5.1/include"

        "Engine/src",
        "vendor",
        "%{prj.name}/src",
        "%{prj.name}/src/scripts"
    }

    links {
        "Engine"
    }

    filter "system:windows"
        cppdialect "C++20"
        staticruntime "On"
        systemversion "latest"

    filter "configurations:Debug" 
        defines {
            "_DEBUG",
            "_WINDOWS",
            "GAME_EXPORTS",
            "_USRDLL"
        }
        symbols "On"
    
    filter "configurations:Release" 
        defines {
            "NDEBUG",
            "_WINDOWS",
            "_USRDLL",
            "GAME_IS_EXPORT"
        }
        optimize "On"
    
    filter "configurations:Dev" 
        defines {
            "_DEBUG",
            "_WINDOWS",
            "GAME_IS_EXPORT",
            "_USRDLL"
        }
        symbols "On"

project "ToadRunner"
    location "ToadRunner"
    kind "ConsoleApp"
    language "C++"

    targetdir ("bin/" .. output_dir .. "/%{prj.name}")
    objdir ("bin-intermediate/" .. output_dir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h"
    }

    includedirs {
        "%{prj.name}/src",
        "vendor",
        "vendor/sfml-imgui",
        "vendor/SFML-2.5.1/include"

        "Engine/src",
        "vendor",
        "%{prj.name}/src",
        "%{prj.name}/src/scripts"
    }

    links {
        "Engine",
        "Game"
    }

    filter "system:windows"
        cppdialect "C++20"
        staticruntime "On"
        systemversion "latest"

    filter "configurations:Debug" 
        defines {
            "_DEBUG",
            "_CONSOLE",
        }
        symbols "On"

    filter "configurations:Release" 
        defines {
            "NDEBUG",
            "_CONSOLE",
        }
        optimize "On"

    filter "configurations:Dev" 
        defines {
            "_DEBUG",
            "_CONSOLE",
            "_TOAD_EDITOR"
        }
        symbols "On"
        