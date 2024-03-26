-- premake5.lua

workspace "Onion"
    architecture "x64"
    configurations { 
        "Debug", 
        "Release", 
        "ReleaseNoEditor",
        "DebugNoEditor",
        "Distro",
    }

    startproject "ToadRunner"
    
output_dir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Engine"
    location "Engine"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"

    targetdir ("bin/" .. output_dir .. "/")
    objdir ("bin-intermediate/" .. output_dir .. "/")

    files {
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.mm",
        "vendor/imgui/**.cpp",
        "vendor/sfml-imgui/imgui-SFML.cpp",
    }

    removefiles {
        "vendor/imgui/examples/**",
        "vendor/imgui/misc/**",
        "vendor/imgui/backends/**",
    }

    includedirs {
        "%{prj.name}/src",
        "Game/src",
        "vendor",
        "vendor/imgui",
        "vendor/sfml-imgui",
        "vendor/SFML-2.6.0/include",
        "vendor/json/include"
    }

    libdirs {
        "vendor/SFML-2.6.0/lib"
    }

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

        pchheader "pch.h"
        pchsource "%{prj.name}/src/pch.cpp"

    filter "files:vendor/**.cpp"
        flags {"NoPCH"}

    filter "system:windows"
        staticruntime "Off"
        systemversion "latest"

    -- configurations 

    filter "configurations:Debug" 
        defines {
            "_DEBUG",
            "_CONSOLE",
            "ENGINE_IS_EXPORT",
            "TOAD_EDITOR"
        }

        runtime "Debug"
        symbols "On"
    
    filter "configurations:Release" 
        defines {
            "NDEBUG",
            "ENGINE_IS_EXPORT",
            "TOAD_EDITOR"
        }

        runtime "Release"
        optimize "On"

    filter "configurations:Distro"
        defines {
            "NDEBUG",
            "ENGINE_IS_EXPORT",
            "TOAD_EDITOR",
            "TOAD_DISTRO",
        }

        runtime "Release"
        optimize "On"
        
    filter "configurations:ReleaseNoEditor"
        defines {
            "NDEBUG",
            "ENGINE_IS_EXPORT",
        }

        runtime "Release"
        optimize "On"

    filter "configurations:DebugNoEditor"
        defines {
            "_DEBUG",
            "_CONSOLE",
            "ENGINE_IS_EXPORT",
        }

        runtime "Debug"
        symbols "On"

    filter "system:macosx"
        links {
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }
    
    filter {"system:windows", "configurations:DebugNoEditor or configurations:Debug"}
        links {
            "sfml-system-d",
            "sfml-window-d",
            "sfml-graphics-d",
            "sfml-audio-d",
        }
    filter {"system:windows", "configurations:ReleaseNoEditor or configurations:Release or configurations:Distro"}
        links {
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }

project "Game"
    location "Game"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"

    targetdir ("bin/" .. output_dir .. "/")
    objdir ("bin-intermediate/" .. output_dir .. "/")

    files {
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h",
        "vendor/imgui/**.cpp",
        "vendor/sfml-imgui/imgui-SFML.cpp",
    }

    removefiles {
        "vendor/imgui/examples/**",
        "vendor/imgui/misc/**",
        "vendor/imgui/backends/**",
    }

    includedirs {
        "Engine/src",
        "vendor",
        "vendor/imgui",
        "vendor/SFML-2.6.0/include",
        "vendor/json/include", 
        "%{prj.name}/src",
        "%{prj.name}/src/scripts"
    }

    libdirs {
        "vendor/SFML-2.6.0/lib"
    }

    links {
        "Engine",
    }

    filter "system:macosx"
        links {
            "OpenGL.framework",
            "Cocoa.framework",
            "IOKit.framework",
            "CoreVideo.framework",
        }

    filter "system:windows"
        links{
            "opengl32"
        }

    filter "system:windows"
        staticruntime "Off"
        systemversion "latest"

    -- configurations
    
    filter "configurations:Debug" 
        defines {
            "_DEBUG",
            "_WINDOWS",
            "GAME_IS_EXPORT",
            "TOAD_EDITOR",
            "_USRDLL"
        }

        runtime "Debug"
        symbols "On"

    filter "configurations:Release" 
        defines {
            "NDEBUG",
            "_WINDOWS",
            "GAME_IS_EXPORT",
            "TOAD_EDITOR",
            "_USRDLL"
        }

        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "configurations:Distro" 
        defines {
            "NDEBUG",
            "_WINDOWS",
            "GAME_IS_EXPORT",
            "TOAD_EDITOR",
            "TOAD_DISTRO",
            "_USRDLL"
        }

        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "configurations:ReleaseNoEditor"
        defines {
            "NDEBUG",
            "_WINDOWS",
            "_USRDLL",
            "GAME_IS_EXPORT"
        }

        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "configurations:DebugNoEditor"
        defines {
                "_DEBUG",
                "_WINDOWS",
                "GAME_IS_EXPORT",
                "_USRDLL"
        }

        runtime "Debug"
        symbols "On"

    filter "system:macosx"
        links {
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }
    
    filter {"system:windows", "configurations:DebugNoEditor or configurations:Debug"}
        links {
            "sfml-system-d",
            "sfml-window-d",
            "sfml-graphics-d",
            "sfml-audio-d",
        }
    filter {"system:windows", "configurations:ReleaseNoEditor or configurations:Release or configurations:Distro"}
        links {
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }

project "ToadRunner"
    location "ToadRunner"
    -- filter "configurations:ReleaseNoEditor"
        -- kind "WindowedApp"
    -- filter "configurations:DebugNoEditor or configurations:Debug or configurations:Release or configurations:Distro"
    kind "ConsoleApp"
    -- filter {}
    language "C++"
    cppdialect "C++20"

    targetdir ("bin/" .. output_dir .. "/")
    objdir ("bin-intermediate/" .. output_dir .. "/")

    files {
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.mm",
        "vendor/imgui/**.cpp",
        "vendor/sfml-imgui/imgui-SFML.cpp",
        "vendor/pugixml/**.cpp",
    }

    removefiles {
        "vendor/imgui/examples/**",
        "vendor/imgui/misc/**",
        "vendor/imgui/backends/**",
    }

    includedirs {
        "%{prj.name}/src",
        "%{prj.name}/src/scripts",
        "Engine/src",
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
        "Game",
    }

    filter "system:macosx"
         links {
             "OpenGL.framework",
             "Cocoa.framework",
             "IOKit.framework",
             "CoreVideo.framework",
         }
    filter "system:windows"
        links{
            "opengl32"
        }

        pchheader "pch.h"
        pchsource "%{prj.name}/src/pch.cpp"

    filter "files:vendor/**.cpp"
        flags {"NoPCH"}
    
    filter "files:ToadRunner/src/**.mm"
        flags {"NoPCH"}
        
    filter "system:windows"
        staticruntime "Off"
        systemversion "latest"

    -- configurations 

    filter "configurations:Debug" 
        defines {
            "_DEBUG",
            "_CONSOLE",
            "TOAD_EDITOR"
        }

        runtime "Debug"
        symbols "On"

    filter "configurations:Release" 
        defines {
            "NDEBUG",
            "_CONSOLE",
            "TOAD_EDITOR",
        }

        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "configurations:Distro" 
        defines {
            "NDEBUG",
            "_CONSOLE",
            "TOAD_EDITOR",
            "TOAD_DISTRO",
        }

        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "configurations:ReleaseNoEditor"
        defines {
            "NDEBUG",
        }

        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "configurations:DebugNoEditor"
        defines {
            "_DEBUG",
            "_CONSOLE",
        }

        runtime "Debug"
        symbols "On"

    filter "system:macosx"
        links {
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }
    
    filter {"system:windows", "configurations:DebugNoEditor or configurations:Debug"}
        links {
            "sfml-system-d",
            "sfml-window-d",
            "sfml-graphics-d",
            "sfml-audio-d",
        }
    filter {"system:windows", "configurations:ReleaseNoEditor or configurations:Release or configurations:Distro"}
        links {
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }